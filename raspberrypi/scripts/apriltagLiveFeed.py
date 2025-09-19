import io
import logging
import socket
import socketserver
from http import server
from threading import Condition
import cv2
import numpy as np
from pupil_apriltags import Detector
import picamera2
from picamera2.encoders import JpegEncoder
from picamera2.outputs import FileOutput
from libcamera import Transform
import RPi.GPIO as GPIO
import pigpio
import threading
import multiprocessing
from multiprocessing import Process, Queue

# Set up the servo motor control
servo_pin = 18
pwm = pigpio.pi()
pwm.set_mode(servo_pin, pigpio.OUTPUT)
pwm.set_PWM_frequency(servo_pin, 50)

# Initial pulse width
pw = 1450
pwm.set_servo_pulsewidth(servo_pin, pw)

# HTML page with JS to detect arrow key presses and send POST requests
PAGE = """\
<html>
<head>
<title>Picamera2 MJPEG Streaming with AprilTag Detection</title>
<script type="text/javascript">
    document.addEventListener('keydown', function(event) {
        var key;
        if (event.key === 'ArrowUp') {
            key = 'up';
        } else if (event.key === 'ArrowDown') {
            key = 'down';
        } else if (event.key === 'ArrowLeft') {
            key = 'left';
        } else if (event.key === 'ArrowRight') {
            key = 'right';
        }

        if (key) {
            var xhr = new XMLHttpRequest();
            xhr.open("POST", "/control", true);
            xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
            xhr.send("key=" + key);
        }
    });
</script>
</head>
<body>
<h1>Picamera2 MJPEG Streaming with AprilTag Detection</h1>
<img src="stream.mjpg" width="640" height="480" />
<p>Use the arrow keys to control the camera.</p>
</body>
</html>
"""

# Buffer for streaming the camera frames
class StreamingOutput:
    def __init__(self):
        self.frame = None
        self.condition = Condition()
        
    def set_frame(self, frame):
        with self.condition:
            self.frame = frame
            self.condition.notify_all()

# HTTP request handler
class StreamingHandler(server.BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/':
            self.send_response(301)
            self.send_header('Location', '/index.html')
            self.end_headers()
        elif self.path == '/index.html':
            content = PAGE.encode('utf-8')
            self.send_response(200)
            self.send_header('Content-Type', 'text/html')
            self.send_header('Content-Length', len(content))
            self.end_headers()
            self.wfile.write(content)
        elif self.path == '/stream.mjpg':
            self.send_response(200)
            self.send_header('Age', 0)
            self.send_header('Cache-Control', 'no-cache, private')
            self.send_header('Pragma', 'no-cache')
            self.send_header('Content-Type', 'multipart/x-mixed-replace; boundary=FRAME')
            self.end_headers()
            try:
                while True:
                    with output.condition:
                        output.condition.wait()
                        frame = output.frame
                    self.wfile.write(b'--FRAME\r\n')
                    self.send_header('Content-Type', 'image/jpeg')
                    self.send_header('Content-Length', len(frame))
                    self.end_headers()
                    self.wfile.write(frame)
                    self.wfile.write(b'\r\n')
            except Exception as e:
                logging.warning(
                    'Removed streaming client %s: %s',
                    self.client_address, str(e))
        else:
            self.send_error(404)
            self.end_headers()
            
    def do_POST(self):
        global pw
        if self.path == '/control':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length).decode('utf-8')
            key = post_data.split('=')[1]
            moveSize = 10

            # Adjust the pulse width (pw) based on key press
            if key == 'left':
                if pw < 1525:  # Move right within bounds
                    pw += moveSize
            elif key == 'right':
                if pw > 1380:  # Move left within bounds
                    pw -= moveSize
            
            # Set the pulse width to control the servo
            pwm.set_servo_pulsewidth(servo_pin, pw)
            print(f"Key pressed: {key}, Pulsewidth: {pw}")

            self.send_response(200)
            self.end_headers()

# HTTP server setup
class StreamingServer(socketserver.ThreadingMixIn, server.HTTPServer):
    allow_reuse_address = True
    daemon_threads = True

# Function to get the Raspberry Pi's IP address
def get_ip_address():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        # Doesn't even have to be reachable
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
    except Exception:
        ip = "localhost"
    finally:
        s.close()
    return ip

def apriltag_process(frame_queue, result_queue, camera_params, tag_size):
    # Initialize the AprilTag detector
    at_detector = Detector(
        families="tag36h11",
        nthreads=4,  # Use all 4 cores
        quad_decimate=2.0,  # Increase for faster processing
        quad_sigma=0.0,
        refine_edges=1,
        decode_sharpening=0.25,
        debug=0
    )
    while True:
        frame = frame_queue.get()
        if frame is None:
            break
        # Convert the frame to grayscale
        img_gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        # Detect the AprilTags in the frame
        tags = at_detector.detect(img_gray, estimate_tag_pose=True, camera_params=camera_params, tag_size=tag_size)
        result_queue.put((frame, tags))

def main():
    global output, pwm, pw

    output = StreamingOutput()

    # Define camera parameters for pose estimation
    fx = 300  # Adjusted focal length
    fy = 300
    cx = 160  # Adjusted principal point
    cy = 120
    camera_params = [fx, fy, cx, cy]
    tag_size = 0.16  # Size of the AprilTag in meters (adjust to your tag size)

    # Queues for inter-process communication
    frame_queue = Queue(maxsize=2)
    result_queue = Queue(maxsize=2)

    # Start the AprilTag detection process
    process = Process(target=apriltag_process, args=(frame_queue, result_queue, camera_params, tag_size))
    process.start()

    # Camera setup with reduced resolution
    picam2 = picamera2.Picamera2()
    config = picam2.create_video_configuration(
        main={"size": (320, 240), "format": "RGB888"},
        transform=Transform(vflip=True, hflip=True)
    )
    picam2.configure(config)

    # Start the camera
    picam2.start()

    # Start a thread to handle frame capture and processing
    def capture_frames():
        while True:
            frame = picam2.capture_array()
            if not frame_queue.full():
                frame_queue.put(frame)
            # Check for results and overlay detections
            if not result_queue.empty():
                processed_frame, tags = result_queue.get()
                # Draw detections on the frame
                for tag in tags:
                    corners = tag.corners.astype(int)
                    cv2.polylines(processed_frame, [corners], True, (0, 255, 0), thickness=2)
                    cX = int((corners[0, 0] + corners[2, 0]) / 2)
                    cY = int((corners[0, 1] + corners[2, 1]) / 2)
                    # Calculate the rotation of the tag
                    dx = corners[2, 0] - corners[0, 0]
                    dy = corners[2, 1] - corners[0, 1]
                    rotation = np.degrees(np.arctan2(dy, dx))
                    rotation = (rotation + 45) % 360  # Adjust the rotation by adding 45 degrees

                    # Put the center coordinates near the tag center
                    center_text = "x={:d}, y={:d}".format(cX, cY)
                    cv2.putText(processed_frame, center_text, (cX, cY + 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

                    # Display the tag id
                    id_text = "id={:d}".format(tag.tag_id)
                    cv2.putText(processed_frame, id_text, (cX, cY - 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

                    # Display the rotation
                    rotation_text = "rotation={:.2f} deg".format(rotation)
                    cv2.putText(processed_frame, rotation_text, (cX, cY - 40), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

                    # Get the pose estimation (translation vector)
                    if tag.pose_t is not None:
                        t = tag.pose_t  # Translation vector
                        z = t[2][0]  # Distance along Z-axis (forward from camera)
                        z_text = "z={:.2f} m".format(z)
                        cv2.putText(processed_frame, z_text, (cX, cY + 40), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

                # Encode the frame as JPEG
                ret, jpeg = cv2.imencode('.jpg', processed_frame)
                if ret:
                    output.set_frame(jpeg.tobytes())
            else:
                # If no new detection results, use the original frame
                ret, jpeg = cv2.imencode('.jpg', frame)
                if ret:
                    output.set_frame(jpeg.tobytes())

    threading.Thread(target=capture_frames, daemon=True).start()

    try:
        address = ('', 8000)
        ip = get_ip_address()
        print(f"\n\n\nStarting server at http://{ip}:8000\n\n\n")
        server = StreamingServer(address, StreamingHandler)
        server.serve_forever()
    finally:
        picam2.stop()
        process.terminate()
        process.join()
        pwm.set_PWM_dutycycle(servo_pin, 0)
        pwm.set_PWM_frequency(servo_pin, 0)

if __name__ == '__main__':
    main()

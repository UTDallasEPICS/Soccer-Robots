# Import necessary modules
import io
import logging
import socket
import socketserver
from http import server
from threading import Condition
import threading
import multiprocessing
from multiprocessing import Process, Queue
import cv2
import numpy as np
from pupil_apriltags import Detector
import picamera2
from picamera2.encoders import JpegEncoder
from picamera2.outputs import FileOutput
from libcamera import Transform
import time

# HTML page with movement commands reference and charging status message
PAGE = """\
<html>
<head>
<title>Picamera2 MJPEG Streaming with AprilTag Detection</title>
<script type="text/javascript">
    document.addEventListener('DOMContentLoaded', function() {
        var eventSource = new EventSource('/events');
        eventSource.onmessage = function(e) {
            document.getElementById('status_message').textContent = e.data;
        };
    });
</script>
<style>
    body { font-family: Arial, sans-serif; }
    #commands { margin-top: 20px; }
</style>
</head>
<body>
<h1>Picamera2 MJPEG Streaming with AprilTag Detection</h1>
<img src="stream.mjpg" width="640" height="480" />
<p id="status_message">Loading...</p>
<div id="commands">
    <h3>Movement Commands Reference:</h3>
    <ul>
        <li>1: Move Forward</li>
        <li>2: Move Backward</li>
        <li>3: Rotate Left</li>
        <li>4: Rotate Right</li>
        <li>0: Stop</li>
    </ul>
</div>
</body>
</html>
"""

# Buffer for streaming the camera frames
class StreamingOutput:
    def __init__(self):
        self.frame = None
        self.condition = Condition()
        self.clients = []

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
        elif self.path == '/events':
            # SSE endpoint for sending status messages
            self.send_response(200)
            self.send_header('Content-Type', 'text/event-stream')
            self.send_header('Cache-Control', 'no-cache')
            self.end_headers()
            try:
                while True:
                    message = status_message
                    self.wfile.write(f"data: {message}\n\n".encode('utf-8'))
                    time.sleep(1)
            except Exception as e:
                pass
        else:
            self.send_error(404)
            self.end_headers()

# HTTP server setup
class StreamingServer(socketserver.ThreadingMixIn, server.HTTPServer):
    allow_reuse_address = True
    daemon_threads = True

# Function to compute movement instruction based on the current state
def compute_movement(car_state, cX, cY, rotation, targetX, targetY, desired_rotation):
    # Compute vector to target
    deltaX = targetX - cX
    deltaY = targetY - cY

    # Desired angle to target
    desired_angle = (np.degrees(np.arctan2(deltaY, deltaX)) + 360) % 360

    # Corrected angle difference calculation
    angle_diff = ((rotation - desired_angle + 180) % 360) - 180

    # Distance to target
    distance = np.hypot(deltaX, deltaY)

    # Thresholds
    angle_threshold = 15  # Increased to prevent oscillations
    distance_threshold = 20  # Adjusted based on testing
    rotation_threshold = 15  # Increased for final orientation

    # Movement commands
    # 1: Move Forward
    # 2: Move Backward
    # 3: Rotate Left
    # 4: Rotate Right
    # 0: Stop

    if car_state['state'] == 0:
        # Rotate to face the target
        if angle_diff < -angle_threshold:
            movement = 3  # Rotate Left
        elif angle_diff > angle_threshold:
            movement = 4  # Rotate Right
        else:
            movement = 0
            car_state['state'] = 1
    elif car_state['state'] == 1:
        # Move towards the target
        if distance > distance_threshold:
            movement = 1
        else:
            movement = 0
            car_state['state'] = 2
    elif car_state['state'] == 2:
        # Rotate to desired final orientation
        rotation_diff = ((rotation - desired_rotation + 180) % 360) - 180
        if rotation_diff < -rotation_threshold:
            movement = 3  # Rotate Left
        elif rotation_diff > rotation_threshold:
            movement = 4  # Rotate Right
        else:
            movement = 0
            car_state['state'] = 3
    else:
        # Arrived and correctly oriented
        movement = 0

    # Logging for debugging
    print(f"State: {car_state['state']}, Movement: {movement}, Desired Angle: {desired_angle:.2f}, Rotation: {rotation:.2f}, Angle Diff: {angle_diff:.2f}")

    return movement

# Process to handle AprilTag detection
def apriltag_process(frame_queue, result_queue, camera_params, tag_size):
    # Initialize the AprilTag detector
    at_detector = Detector(
        families="tag36h11",
        nthreads=4,  # Use all 4 cores
        quad_decimate=2.0,  # Adjust for performance
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
        # Detect the AprilTags in the frame with pose estimation
        tags = at_detector.detect(img_gray, estimate_tag_pose=True, camera_params=camera_params, tag_size=tag_size)
        result_queue.put((frame, tags))

def main():
    global output, camera_params, car_states, charging_stations, moving_car_id, status_message

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

    # Original resolution
    original_width = 640
    original_height = 480

    # Camera setup with reduced resolution
    current_width = 320
    current_height = 240
    picam2 = picamera2.Picamera2()
    config = picam2.create_video_configuration(
        main={"size": (current_width, current_height), "format": "RGB888"},
        transform=Transform(vflip=True, hflip=True)
    )
    picam2.configure(config)

    # Calculate scaling factors
    scale_x = current_width / original_width
    scale_y = current_height / original_height

    # Adjusted charging stations coordinates
    charging_stations = {
        0: {
            'x_range': (int(20 * scale_x), int(120 * scale_x)),
            'y_range': (int(190 * scale_y), int(290 * scale_y)),
            'targetX': int(70 * scale_x),
            'targetY': int(240 * scale_y),
            'desired_rotation': 90,
            'color': (0, 0, 255)
        },
        1: {
            'x_range': (int(520 * scale_x), int(620 * scale_x)),
            'y_range': (int(190 * scale_y), int(290 * scale_y)),
            'targetX': int(570 * scale_x),
            'targetY': int(240 * scale_y),
            'desired_rotation': 270,
            'color': (0, 0, 255)
        }
    }

    # Initialize car states (state machine for each car)
    car_states = {}

    # Initialize moving car ID and status message
    moving_car_id = None
    status_message = "Loading..."

    # Initialize rotation history for smoothing rotations
    rotation_history = {}

    # Start the camera
    picam2.start()

    # Start a thread to handle frame capture and processing
    def capture_frames():
        global moving_car_id, status_message
        while True:
            frame = picam2.capture_array()
            if not frame_queue.full():
                frame_queue.put(frame)
            # Check for results and process detections
            if not result_queue.empty():
                frame, tags = result_queue.get()

                # Initialize a dictionary to store car positions and rotations
                car_positions = {}

                # For each detected tag, draw bounding boxes and display information
                for tag in tags:
                    # Get the corners of the tag
                    corners = tag.corners.astype(int)

                    # Draw a green bounding box around the tag
                    cv2.polylines(frame, [corners], True, (0, 255, 0), thickness=2)

                    # Get the tag center
                    cX = int(np.mean(corners[:, 0]))
                    cY = int(np.mean(corners[:, 1]))

                    # Put the center coordinates near the tag center
                    center_text = "x={:d}, y={:d}".format(cX, cY)
                    cv2.putText(frame, center_text, (cX, cY + int(20 * scale_y)), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

                    # Display the tag id
                    id_text = "id={:d}".format(tag.tag_id)
                    cv2.putText(frame, id_text, (cX, cY - int(20 * scale_y)), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

                    # Corrected Rotation Calculation using pose estimation
                    # Extract rotation matrix
                    R = tag.pose_R

                    # Calculate Euler angles from rotation matrix
                    def rotationMatrixToEulerAngles(R):
                        sy = np.sqrt(R[0, 0] ** 2 + R[1, 0] ** 2)
                        singular = sy < 1e-6
                        if not singular:
                            x_angle = np.arctan2(R[2, 1], R[2, 2])
                            y_angle = np.arctan2(-R[2, 0], sy)
                            z_angle = np.arctan2(R[1, 0], R[0, 0])
                        else:
                            x_angle = np.arctan2(-R[1, 2], R[1, 1])
                            y_angle = np.arctan2(-R[2, 0], sy)
                            z_angle = 0
                        return np.degrees(z_angle)

                    rotation = rotationMatrixToEulerAngles(R)
                    rotation = (rotation + 360) % 360  # Normalize to [0, 360)

                    # Smoothing the rotation values
                    if tag.tag_id not in rotation_history:
                        rotation_history[tag.tag_id] = []
                    rotation_history[tag.tag_id].append(rotation)
                    # Keep only the last N values
                    if len(rotation_history[tag.tag_id]) > 5:
                        rotation_history[tag.tag_id].pop(0)
                    # Compute the average rotation
                    smoothed_rotation = sum(rotation_history[tag.tag_id]) / len(rotation_history[tag.tag_id])

                    # Display the rotation
                    rotation_text = "rotation={:.2f} deg".format(smoothed_rotation)
                    cv2.putText(frame, rotation_text, (cX, cY - int(40 * scale_y)), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

                    # Store the position and rotation
                    car_positions[tag.tag_id] = {'cX': cX, 'cY': cY, 'rotation': smoothed_rotation}

                # Initialize the charging status
                all_cars_at_charging = True
                cars_not_at_charging = []

                # Process each charging station
                for tag_id, station in charging_stations.items():
                    # Draw the charging station rectangle
                    x1, x2 = station['x_range']
                    y1, y2 = station['y_range']
                    color = station['color']
                    cv2.rectangle(frame, (x1, y1), (x2, y2), color, 2)
                    # Draw a circle at the center of the charging station
                    cv2.circle(frame, (station['targetX'], station['targetY']), 5, (255, 255, 0), -1)
                    # Label the charging station
                    cv2.putText(frame, f"Charging Station {tag_id}", (x1, y1 - int(10 * scale_y)), cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 2)

                    # Check if the car is detected
                    if tag_id in car_positions:
                        cX = car_positions[tag_id]['cX']
                        cY = car_positions[tag_id]['cY']
                        rotation = car_positions[tag_id]['rotation']
                        desired_rotation = station['desired_rotation']

                        # Calculate distance to charging station center
                        distance = np.hypot(station['targetX'] - cX, station['targetY'] - cY)
                        rotation_diff = ((rotation - desired_rotation + 180) % 360) - 180

                        if distance <= 10 and abs(rotation_diff) <= 5:
                            # Car is at charging station and oriented correctly
                            station['color'] = (0, 255, 0)
                            car_states[tag_id] = {'state': 3}  # Update state to arrived
                        else:
                            # Car is not at charging station or not oriented correctly
                            station['color'] = (0, 0, 255)
                            all_cars_at_charging = False
                            cars_not_at_charging.append(tag_id)
                    else:
                        # Car not detected
                        station['color'] = (0, 0, 255)
                        all_cars_at_charging = False
                        cars_not_at_charging.append(tag_id)

                # Update status message
                status_message = "Cars At Charging Station" if all_cars_at_charging else "Cars Not At Charging Station"

                # Navigation logic
                if not all_cars_at_charging:
                    if moving_car_id is None or moving_car_id not in cars_not_at_charging:
                        # Select next car to move
                        moving_car_id = cars_not_at_charging[0]
                        # Initialize the car's state machine
                        if moving_car_id not in car_states or car_states[moving_car_id]['state'] == 3:
                            car_states[moving_car_id] = {'state': 0}

                    if moving_car_id in car_positions:
                        # Get current car position and state
                        cX = car_positions[moving_car_id]['cX']
                        cY = car_positions[moving_car_id]['cY']
                        rotation = car_positions[moving_car_id]['rotation']
                        station = charging_stations[moving_car_id]
                        targetX = station['targetX']
                        targetY = station['targetY']
                        desired_rotation = station['desired_rotation']
                        car_state = car_states[moving_car_id]

                        # Compute movement instruction
                        movement = compute_movement(car_state, cX, cY, rotation, targetX, targetY, desired_rotation)

                        # Print movement instruction
                        print(f"ID: {moving_car_id}, Movement: {movement}, State: {car_state['state']}, Rotation: {rotation:.2f}")

                        # Display movement instruction on the frame
                        cv2.putText(frame, f"ID: {moving_car_id}, Movement: {movement}, State: {car_state['state']}", (10, int(20 * scale_y)), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 2)

                        # TODO: Send the movement command to the car (e.g., via serial communication)
                        # For now, we just print it
                    else:
                        # Car not detected, cannot move
                        pass
                else:
                    moving_car_id = None  # Reset moving car ID

                # Encode the frame as JPEG
                ret, jpeg = cv2.imencode('.jpg', frame)
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

if __name__ == '__main__':
    main()
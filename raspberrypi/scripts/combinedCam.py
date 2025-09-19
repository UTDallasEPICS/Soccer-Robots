import io
import logging
import socket
import socketserver
import picamera2
from http import server
from threading import Condition
from picamera2.encoders import JpegEncoder
from picamera2.outputs import FileOutput
from libcamera import Transform 
import RPi.GPIO as GPIO
from time import sleep
import pigpio

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
<title>picamera2 MJPEG streaming demo</title>
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
<h1>Picamera2 MJPEG Streaming Demo</h1>
<img src="stream.mjpg" width="640" height="480" />
<p>Use the arrow keys to control the camera.</p>
</body>
</html>
"""

# Buffer for streaming the camera
class StreamingOutput(io.BufferedIOBase):
    def __init__(self):
        self.frame = None
        self.condition = Condition()

    def write(self, buf):
        with self.condition:
            self.frame = buf
            self.condition.notify_all()

# HTTP Server setup
class StreamingServer(socketserver.ThreadingMixIn, server.HTTPServer):
    allow_reuse_address = True
    daemon_threads = True

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

# Function to get the Raspberry Pi's IP address
def get_ip_address():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        # doesn't even have to be reachable
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
    except Exception:
        ip = "localhost"
    finally:
        s.close()
    print("Ip: " + str(ip));
    return ip

# Camera setup (have to flip vertically and horizontally because of the camera's position)
picam2 = picamera2.Picamera2()
config = picam2.create_video_configuration(
    main={"size": (640, 480)},
    transform=Transform(vflip=True, hflip=True)  # Apply both vertical and horizontal flips
)
picam2.configure(config)
output = StreamingOutput()
picam2.start_recording(JpegEncoder(), FileOutput(output))


# Start the server
try:
    address = ('', 8000)
    ip = get_ip_address()
    print(f"\n\n\nStarting server at http://{ip}:8000\n\n\n")
    server = StreamingServer(address, StreamingHandler)
    server.serve_forever()
finally:
    picam2.stop_recording()
    pwm.set_PWM_dutycycle(servo_pin, 0)
    pwm.set_PWM_frequency(servo_pin, 0)

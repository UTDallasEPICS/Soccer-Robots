# For refresh rate and executing shell commands
import time
import subprocess

WIDTH = 128
HEIGHT = 64

# Initialize I2C bus
import board
import busio

# Create I2C instance with default SCL and SDA pins
i2c = busio.I2C(board.SCL, board.SDA)

# Create SSD1306 Driver, specifying width, height, and i2c instance
import adafruit_ssd1306
oled = adafruit_ssd1306.SSD1306_I2C(WIDTH, HEIGHT, i2c)

# Define the reset line
import digitalio
oled_reset = digitalio.DigitalInOut(board.D4)

# Clear Display
oled.fill(0)
oled.show()

# Create blank image for drawing with 1 bit color
from PIL import Image, ImageDraw, ImageFont
image = Image.new("1", (oled.width, oled.height))

# Get drawing object to draw on image
draw = ImageDraw.Draw(image)

while True:
    # Draw border
    draw.rectangle((0, 0, oled.width-1, oled.height-1), outline=255, fill=0)

    # Load default font
    font = ImageFont.load_default()

    cmd = "hostname -I |cut -f 2 -d ' '"
    text = "wlan1: " + str(subprocess.check_output(cmd, shell = True), 'utf-8')
    bbox = font.getbbox(text)
    (font_width, font_height) = bbox[2] - bbox[0], bbox[3] - bbox[1]
    draw.text(
        (oled.width//2 - font_width//2, 3),
        text,
        font=font,
        fill = 255,
    )

    cmd = "hostname -I |cut -f 1 -d ' '"
    text = "wlan0: " + str(subprocess.check_output(cmd, shell = True), 'utf-8')
    bbox = font.getbbox(text)
    (font_width, font_height) = bbox[2] - bbox[0], bbox[3] - bbox[1]
    draw.text(
        (oled.width//2 - font_width//2, 15),
        text,
        font=font,
        fill = 255,
    )

    cmd = "vcgencmd measure_temp |cut -f 2 -d '='"
    text = "TEMP: " + str(subprocess.check_output(cmd, shell = True), 'utf-8')
    bbox = font.getbbox(text)
    (font_width, font_height) = bbox[2] - bbox[0], bbox[3] - bbox[1]
    draw.text(
        (oled.width//2 - font_width//2, 25),
        text,
        font=font,
        fill = 255,
    )

    cmd = "free -m | awk 'NR==2{printf \"%s/%sMB %.2f%%\", $3,$2,$3*100/$2 }'"
    text = str(subprocess.check_output(cmd, shell = True), 'utf-8')
    bbox = font.getbbox(text)
    (font_width, font_height) = bbox[2] - bbox[0], bbox[3] - bbox[1]
    draw.text(
        (oled.width//2 - font_width//2, 35),
        text,
        font=font,
        fill = 255,
    )

    cmd = "vcgencmd get_throttled | cut -f 2 -d '='"
    text = str(subprocess.check_output(cmd, shell = True), 'utf-8')
    match text:
        case "0x0\n":
            text = ""
        case "0x1\n":
            text = "UV DETECT"
        case "0x2\n":
            text = "FREQ CAPPED"
        case "0x4\n":
            text = "THROTTLING"
        case "0x8\n":
            text = "SOFT TEMP LIM"
        case "0x10000\n":
            text = "UV OCCUR"
        case "0x20000\n":
            text = "FREQ CAP OCCUR"
        case "0x80000\n":
            text = "OSFT TEMP LIM OCCUR"
    bbox = font.getbbox(text)
    (font_width, font_height) = bbox[2] - bbox[0], bbox[3] - bbox[1]
    draw.text(
        (oled.width//2 - font_width//2, 45),
        text,
        font=font,
        fill = 255,
    )

    oled.image(image)
    oled.show()
    time.sleep(1)

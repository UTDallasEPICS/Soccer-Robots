# ESP32 Robots

The robots are run on:
- WEMOS **ESP32-S2-Mini** Development Board
- Repeat Robotics **Budget Ant DESC**, which drives the DC motors
- Repeat Robotics **Antweight Brushed DC Motors**
- Headway LiFePO4 **40152S Battery**
- **MT3608** DC-DC Boost Converter

## Setting Up Environment

Make sure your esp_idf toolchain is installed and you can run idf.py. Follow the steps for installation of ESP-IDF, you can use VS Code's extension for it if needed.
Before we build and flash our project to the microcontroller, we need to tell esp_idf what microcontroller we're flashing to (which is the ESP32-S2).

Run `idf.py set-target esp32s2`

## Setup WiFi Connection

Run `idf.py menuconfig`

1. Navigate to "Connectivity Configuration"  
2. Edit WIFI SSID and Password accordingly.
3. Save and exit.

The ESP32 is set up to do two things primarily: take commands from the raspberry pi and then drive the motors according to those commands.

The ESP has a TCP connection set up with the Raspberry Pi. It does this by connecting to the same wifi as the Raspberry Pi. This should be RPIHotspot, currently the configuration for the wifi and its password may be different so this may need to be changed. Take note of the esp's ip address from the serial monitor; that will be put into the raspberry pi programer so that it knows what address it should connect to.

The Raspberry Pi should send messages which include "readyCheck" when the website wants to see if the robots are ready, "reset" when the game ends, "ignore" for test messages, and different keys to signify different types of movement.

We also use semaphores to balance between the multithreading we do between the movement and the receiving input.

Finally, as for the movement we use basically s-curves for smooth control. The equation I use has the curve looks kind of like this, where "s" and "f" are the different speeds we want to move between.
![image](https://github.com/user-attachments/assets/8f57eb09-9af4-40c4-bd81-6aa076cef75f)

So basically our x value will change over time (which we do with a hardware timer), and our y value will be the new speed we are using. This allows us to smoothly change between different speeds depending on what's being pressed.

One important note is that the correct pulse widths don't seem to really be accurate to what's given on the ESC documentation (which is <https://repeat-robotics.com/buy/desc/>). It might be just with the one ESC we had working though. I would check the new ESC's to see the valid pulse width ranges and valid duty cycles needed. While you'd expect the valid duty cycle %'s to be 36-50 and 86-100, those do get the valid pulse widths, but they don't seem to really work.

Also note you have to initially set the duty cycle to in between reverse and forward, and you have to slowly increase it in either direction. So let's say the valid ranges are in fact 36-50 for reverse and 86-100. You can't just do duty = 90 and expect it to work. You'd say have to do duty = 60, and incement it until it reaches 90 to drive the motors. Likewise in reverse.

To understand a bit better on how motors work, research pulse width modulation, duty cycles, positive pulse width and know about frequency/period.

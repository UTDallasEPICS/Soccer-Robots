### Setting Up Environment
Make sure your esp_idf toolchain is installed and you can run idf.py. Follow the steps for installation of ESP-IDF, you can use VS Code's extension for it if needed.
Before we build and flash our project to the microcontroller, we need to tell esp_idf what microcontroller we're flashing to (which is the ESP32-S2).

Run `idf.py set-target esp32s2`

### Setup WiFi Connection

Run `idf.py menuconfig`

1. Navigate to "Connectivity Configuration"  
2. Edit WIFI SSID and Password accordingly.
3. Save and exit.

The ESP32 is set up to do two things primarily: take commands from the raspberry pi and then drive the motors according to those commands.

The ESP has a TCP connection set up with the Raspberry Pi. It does this by connecting to the same wifi as the Raspberry Pi. This should be RPIHotspot, currently the configuration for the wifi and its password may be different so this may need to be changed. Take note of the esp's ip address from the serial monitor; that will be put into the raspberry pi programer so that it knows what address it should connect to.

The Raspberry Pi should send messages which include "readyCheck" when the website wants to see if the robots are ready, "reset" when the game ends, "ignore" for test messages, and different keys to signify different types of movement.

We also use semaphores to balance between the multithreading we do between the movement and the receiving input.

Finally, as for the movement we use basically s-curves for smooth control. The equation I use has the curve look kind of like this, where "s" and "f" are the different speeds we want to move between.
![image](https://github.com/user-attachments/assets/8f57eb09-9af4-40c4-bd81-6aa076cef75f)

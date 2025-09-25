# Starting the Raspberry Pi

## Setup

Before starting anything, make sure you have installed the Driver for the WI-FI Adapter. You must also install all libraries to use the camera.


### Installing Camera Libraries

You will need libcamera and picamera2 
**sudo apt install libcamera**
**sudo apt install -y python3-picamera2**
[TROUBLE SHOOTING](https://www.youtube.com/watch?time_continue=356&v=U7yVpYv3gxQ&embeds_referring_euri=https%3A%2F%2Fwww.google.com%2Fsearch%3Fq%3Dconnecting%2Bcamera%2Bmodule%2B3%2Bto%2Bpi4%26rlz%3D1C1VDKB_enUS994US994%26oq%3Dconnecting%2Bcamera%2Bmodule%2B3%2Bto%2Bpi4%26&source_ve_path=MzY4NDIsMzY4NDIsMzY4NDIsMzY4NDIsMzY4NDIsMzY4NDIsMzY4NDIsMzY4NDIsMzY4NDIsMTM5MTE3LDIzODUx)


### Installing WI-FI Driver
When testing if the driver is installed type lsusb in the command line to find usb name and position. Type lsusb -t and find usb. look for "Driver=". If there is nothing there like rtl8852bu then the driver is not installed.

If this is the case, find the required chipset.
Install the chipset onto the driver by cloning the driver repo best suited and following the steps.

Ensure you have the right developer tools.
**sudo apt install -y build-essential bc dkms git**

### Starting the hotspot
To broadcast the hotspot, run the following command to create a hotspot, replacing the <hotspot name> and <hotspot password> placeholders with a hotspot name and password of your choice

**$ sudo nmcli device wifi hotspot ssid <hotspot name> password <hotspot password> ifname wlan1**

### Accessing the Raspberry Pi

After turning on the Pi and connecting to the internet, use your terminal to SSH into the Pi by typing <username>@<ipaddress>, where "timthegoat" is the current username and the ipaddress is your local Pi address shown below(NOTE: Once hotspot is setup this will default to 10.42.0.1).
![image](https://github.com/user-attachments/assets/e83135cc-39a2-4ad9-8e8b-7ac2651071df)

NOTE: when attempting to run the servers, you'll need to activate the virtual
environment (which includes things like the libraries used). To activate this,
create a Python (3.11) virtual environment from the requirements.txt file.

Also, make sure when communicating with the server that both **ControllerPi.py** and **GmServerPi.py** are active. Even if you're just testing the GM server, it will appear to not work unless
the controller server is also active.
(Note: Current Python Version 3.11.2)
### Startup
run start_soccer_robot.sh
**bash start_soccer_robot.sh**

If you run the script, you do not have to do the following: 

To run this, you want to first activate **EspManager.py**. This is basically the process that will communicate with the Raspberry Pi's. It does this by creating child processes, and each child
makes a TCP connection with a single ESP, and communicates with the parent back and forth with unnamed pipes. **ControllerPi.py** and **GmServerPi.py** will send relevant information to **EspManager.py**
through another socket connection. Here's a diagram to explain the purpose of each process a bit better (apologies for the poor quality).
<img width="4445" height="3950" alt="better diagram" src="https://github.com/user-attachments/assets/e2b624f2-6dc2-4511-8342-c66bf52103d8" />


Now to run the Raspberry Pi server, we will first run **EspManager.py**, move that to background, then run **GmServerPi.py**, move that to background, then run **ControllerPi.py**, move that to background,
and that will be all you need to do. Note that you **MUST** run it in this order, otherwise because of the order of socket connections it won't work properly.
RECAP:
1. python EspManager.py (next CTRL+Z to pause, then type "bg" to place process in background)
2. python GmServerPi.py (next CTRL+Z to pause, then type "bg" to place process in background)
3. python ControllerPi.py
(You can use "ps" linux command to check the proccesses that are running, it will have a process id.
If one of the python programs have an error, then use the linux command "kill -9 [process-#]")

Next, on the website do the npm run bootgame, npm run bootcontrol (run this after bootgame always) npm run dev and stuff. Now,
get two people to start the game, and profit as you see the robot moving, the updated timer, and the updating score.
RECAP:
1. npm run bootgame
2. npm run bootcontrol
3. npm run dev

As for the camera, you will be able to stream it by running **"cleanCode.py"**, which will begin streaming the camera to http:IP-ADDR-PORT, as long as it's connected to wifi and the camera itself is
connected onto the Raspberry Pi. Note that while we're temporarily using the smaller camera, we want to use the Arducam camera eventually because it has a wider lens and thus can capture the game better.
Research may need to be done on how to implement it with the Arducam, as there were some difficulties doing so.


So overall, all the scripts you'll really need to run the website will be in the "scripts" directory, and you just need the following 5 files: **EspManager.py, ControllerPi.py, GmServerPi.py, ESPClient.py, and cleanCode.py**. ESPClient.py is used to easily handle connections between the ESP and the Pi.

Additionally, we did have code for the camera to detect charging stations, and give commands to the esp32 so that the robot will move towards the charging stations. The code for this may be in "apriltagLiveFeed.py",
ask your mentor for further details.

      
# File Descriptions


## Startup

### EspManager.py

### ControllerPi.py

### GmServerPi.py

### ESPClient.py


# Camera

## cleanCode.py

## combinedCam.py

## apriltagLiveFeed.py




## backtracking.py

## backupTag.py

## carConnect.py

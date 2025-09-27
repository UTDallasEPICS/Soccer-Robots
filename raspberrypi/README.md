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

This takes in data about motor inputs for each robot and signals from the game manager (game start, robot ready check, game end), and sends the appropriate data to each robot.

To connect with each robot, it forks itself (basically like makes a clone of the program that also starts at the same code line where the fork was) based on however many robots there are. If there are 2 robots, for instnace, there will be 2 forks, and thus 2 child processes will be made.

May need to research pipes, forking, piping, and processes to understand this.

Then, when it receives a request for ready connection from GmServerPi.py, it relays that through pipes to its child processes, which then send a message to the robots asking if they're ready. If they are, they should return back their own success message successfully. Then if all robots returned success message, send that to the game manager file which then tells that to the website.

Also, for motor inputs, ControllerPi.py sends its ID alongside the motor inputs. EspManager checks the ID and sends it to the correct child process, which then sends those inputs to the esp32.

It also has shared memory shared with GmServerPi.py, which is where the current game timer is stored. Basically, when the game ends, it puts in the shared memory that the game is over. EspManager reads from this frequently, and when it sees the game is over, it can shut off communication with the robots, and prepare for reconnection.

### ControllerPi.py

This connects with the website's Controller file, and when the website sends inpts from the user, it directs those inputs to EspManager.py. Pretty straighforward, though it also checks if the input the user sends is the same input as before. If it is, then it doesn't send it, as that doesn't change anything.

### GmServerPi.py

This connects with the website's game manager file. It handles and responds to certain messages from the website. For example, if it receives a packet to check if robots are ready, it sends that request to espmanager, gets a response, and sends that response back up to the website.

When it receives game start command, it begeins running the game, waiting until the timer's up, and when it is it puts in shared memeory with GmServer that time is up, to let it know to disconnect the robots. Then it restarts with waiting fora game to start. It also sends the timer to the website, as it actually is what has the decreasing timer.

### ESPClient.py

This is just a class where each object represents a connection with an esp32, used as a helper function. It has functions like trying to connect to the esp, sending data, and receiging data, alongside error handling too.

# Camera

## cleanCode.py

This creates a page that's hosted on the raspberry pi at a ip address and port to stream the camera display to. It initializes with the camera and starts a server to host the stream on, and connects them to the stream.... somehow. Probably when it calls self.wfile.write(frame), where it writes the current frame of the camera.

## combinedCam.py

This is like the file above, but this time it makes it so if there's keyboard presses of WASD on the raspberry pi, it moves the camera lens to show a differnet place of the field. At least, I'm assuming that's the intention.

NOTE: THIS ONE MAY BE OUTDATED; CHECK WITH DAMIAN ON THIS.

## apriltagLiveFeed.py

This is basically exactly like the above, but now if you put an april tag in front of the camera, this code will detect that april tag. The code to detect the april tag will be useful likely, use this as reference when implementing the algorithm to send robots to charging stations. It may be outdated in the same way that combinedCam is outdated.


## backtracking.py

There isn't any file naemd this, I assume it's just backTest.py. This seems to first do the same as above, streaming it but also showing the april tag on the screen. Right now, it puts the movemenetinstruction on the frame, so ideally the only thing that needs to be done is sending it. It seems right now ti's configured to send 2 pieces of data to the motor: first, the direction of movement (rotationg left, rotating right, moving backwards, moving forwards, stopping) as well has how much to rotae in each direciton. Now, main issue will be configuring that to become packets to send to the esp32. Have fun with that! Also note that each car has 4 states. State 0 is rotating to face target, state 1 is moring towards target, state 2 is doing final rotation, and state 3 means finished arriving and rotating at charigng station.

## backupTag.py

This seems to be like a prototype of the one above? It uses a file that doesn't exist anymore (backtracking.py, though an archive of it is at https://github.com/UTDallasEPICS/Soccer-Robots/blob/1eee7dc0c4098b2c6e86b0c17d89b73867ff42a8/raspberrypi/scripts/archive/Backtracking.py.txt#L4). Even so, backupTag.py references a method in backtracking.py "get_direction", that I can't find in the backtracking.py archive file.

## carConnect.py

This is a file that was used to connect to the ESP32. Back when I made the ESPClient.py file I didn't know this one existed, but all it does for now is just connect and send a handshake, and that's it. Now though, it seems we don't need it since ESPClient.py and ESPManager.py does everything this would've already done.

# Starting the Raspberry Pi

## Setup

Before starting anything, make sure you have installed the Driver for the WI-FI Adapter.
**[NEEDS DETAILS]
**
You must also install all libraries to use the camera.
**[NEEDS DETAILS]
**
## Accessing the Raspberry Pi

After turning on the Pi and connecting to the internet, use your terminal to SSH into the Pi by typing <username>@<ipaddress> where "timthegoat" is the username and the ipaddress is your local Pi address shown below.
![image](https://github.com/user-attachments/assets/e83135cc-39a2-4ad9-8e8b-7ac2651071df)

NOTE: when attempting to run the servers, you'll need to activate the virtual
environment (which includes things like the libraries used). To activate this,
find the folder called "my-env". Inside that directory is another folder called "bin",
and inside of that is a file called activate. You need to call "source activate" to activate
the virtual environment.

Also, make sure when communicating with the server that both **ControllerPi.py** and **GmServerPi.py** are active. Even if you're just testing the GM server, it will appear to not work unless
the controller server is also active.

## Startup

To run this, you want to first activate **EspManager.py**. This is basically the process that will communicate with the Raspberry Pi's. It does this by creating child processes, and each child
makes a TCP connection with a single ESP, and communicates with the parent back and forth with unnamed pipes. **ControllerPi.py** and **GmServerPi.py** will send relevant information to **EspManager.py**
through another socket connection. Here's a diagram to explain the purpose of each process a bit better (apologies for the poor quality).
![image](https://github.com/user-attachments/assets/55c1ff86-75d5-41fe-a101-83da67798dfa)

Now to run the Raspberry Pi server, we will first run Espmanager.py, move that to background, then run **GmServerPi.py**, move that to background, then run **ControllerPi.py**, move that to background,
and that will be all you need to do. Note that you **MUST** run it in this order, otherwise because of the order of socket connections it won't work properly.

Next, on the website do the npm run bootgame, npm run bootcontrol (run this after bootgame always) npm run dev and stuff. Now,
get two people to start the game, and profit as you see the robot moving, the updated timer, and the updating score.

As for the camera, you will be able to stream it by running **"cleanCode.py"**, which will begin streaming the camera to http:IP-ADDR-PORT, as long as it's connected to wifi and the camera itself is
connected onto the Raspberry Pi. Note that while we're temporarily using the smaller camera, we want to use the Arducam camera eventually because it has a wider lens and thus can capture the game better.
Research may need to be done on how to implement it with the Arducam, as there were some difficulties doing so.


So overall, all the scripts you'll really need to run the website will be in the "scripts" directory, and you just need the following 5 files: **EspManager.py, ControllerPi.py, GmServerPi.py, ESPClient.py, and cleanedCode.py**. ESPClient.py is used to easily handle connections between the ESP and the Pi.

Additionally, we did have code for the camera to detect charging stations, and give commands to the esp32 so that the robot will move towards the charging stations. The code for this may be in "apriltagLiveFeed.py",
ask your mentor for further details.

      
# File Descriptions

## EspManager.py

## ControllerPi.py

## GmServerPi.py

## ESPClient.py

## cleanCode.py

## apriltagLiveFeed.py



## Backtracking.py

## BackupTag.py

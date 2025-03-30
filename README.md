# File Descriptions

## Backtracking.py

## BackupTag.py

NOTE: when attempting to run the servers, you'll need to activate the virtual
environment (which includes things like the libraries used). To activate this,
find the folder called "my-env". Inside that directory is another folder called "bin",
and inside of that is a file called activate. You need to call "source activate" to activate
the virtual environment.

Also, make sure when communicating with the server that both the controller and GM server (on the
raspberry pi) are active. If you're just testing the GM server, it will appear to not work unless
the controller server is also active.

Anyways here's how we actually run this and have it be connected with the website
First, in demo-scripts take "ControllerPi.py", and run it. Move it to the backgroun
(while its still running) and then run GmServerPi.py, and run that.

Next, on the website do the npm run bootcontrol, npm run bootgame, npm run dev and stuff. Now,
get two people to start the game, and profit as you see the updating timer and score GRAHHHH.

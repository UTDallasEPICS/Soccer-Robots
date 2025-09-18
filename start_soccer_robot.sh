#!/bin/bash

# Move to the project root
# cd "$(Soccer-Robots "$0")" || {
#   echo "Could not move into project directory"
#   exit 1
# }


mkdir -p logs

# Start EspManager.py
echo "Starting EspManager.py"
nohup python scripts/EspManager.py &>logs/esp.txt  &

# Wait for sockets to initialize
sleep 2

# Start GmServerPi.py
echo "Starting GmServerPi.py"
nohup python scripts/GmServerPi.py &>logs/GMServer.txt &

sleep 2

# Start ControllerPi.py
echo "Starting ControllerPi.py."
nohup python scripts/ControllerPi.py &>logs/Controller.txt &


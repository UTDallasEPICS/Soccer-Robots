from Client import Client
import traceback
import socket
import curses
import os

stdscr = curses.initscr()
stdscr.keypad(1)
stdscr.refresh()

client = Client("192.168.250.158", 30000)
client.initialize()

id = 0;
key = ''
while key != ord('q'):
    key = stdscr.getch()
    stdscr.refresh()
    try:
        id = id + 1
        if key == curses.KEY_UP or key == ord('w'):
            client.send("u")
            print(f"{id} u\r")
        elif key == curses.KEY_DOWN or key == ord('s'):
            client.send("d")
            print(f"{id} d\r")
        elif key == curses.KEY_LEFT or key == ord('a'):
            client.send("l")
        elif key == curses.KEY_RIGHT or key == ord('d'):
            client.send("r")
    except Exception as err:
        print(traceback.format_exc())

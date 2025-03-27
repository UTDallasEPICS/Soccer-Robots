import RPi.GPIO as GPIO
from time import sleep
import pigpio
import curses

screen = curses.initscr()
curses.noecho()
curses.cbreak()
screen.keypad(True)

servo_pin = 18

pwm = pigpio.pi()
pwm.set_mode(servo_pin, pigpio.OUTPUT)
pwm.set_PWM_frequency(servo_pin, 50)

pw = 1450
pwm.set_servo_pulsewidth(servo_pin, pw)

def on_press(key):
    print(key) 

def on_release(key):
    print(key)

try:
    while True:
        char = screen.getch()
        if char == ord('q'):
            break
        elif char == curses.KEY_RIGHT:
            if pw != 1525:
                pw = pw + 1
        elif char == curses.KEY_LEFT:
            if pw != 1380:
                pw = pw - 1
        pwm.set_servo_pulsewidth(servo_pin, pw)
        print(f"Pulsewidth: {pw}\r")
except KeyboardInterrupt:
    print("EXIT\r")
finally:
    curses.nocbreak()
    screen.keypad(0)
    curses.echo()
    curses.endwin()
    pwm.set_PWM_dutycycle(servo_pin, 0)
    pwm.set_PWM_frequency(servo_pin, 0)


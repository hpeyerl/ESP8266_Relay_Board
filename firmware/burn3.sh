#!/bin/sh
./esptool.py --port /dev/ttyUSB0 write_flash 0x12000 webpages.espfs
./esptool.py --port /dev/ttyUSB0 --baud 115200 run && minicom ttyUSB0


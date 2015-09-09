#!/bin/sh
./esptool.py --port /dev/ttyUSB0 --baud 115200 write_flash 0x00000 firmware/0x00000.bin
./esptool.py --port /dev/ttyUSB0 --baud 115200 write_flash 0x40000 firmware/0x40000.bin
./esptool.py --port /dev/ttyUSB0 --baud 115200 write_flash 0x12000 webpages.espfs
./esptool.py --port /dev/ttyUSB0 --baud 115200 write_flash 0x3C000 blank.bin
./esptool.py --port /dev/ttyUSB0 --baud 115200 run && minicom ttyUSB0

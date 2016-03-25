ESPTOOL=../../../tools/esptool-ck/esptool
$ESPTOOL -v -cp /dev/ttyUSB0 -cd none -cb 115200 -ca 0x3C000 -cf blank.bin

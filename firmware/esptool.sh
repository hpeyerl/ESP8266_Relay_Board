esptool -v -eo build/httpd.out -bo firmware/0x00000.bin -bs .text -bs .data -bs .rodata -bc -ec -eo  build/httpd.out -es .irom0.text firmware/0x40000.bin -ec
esptool -v -cp /dev/ttyUSB0 -cd none -cb 115200 -ca 0x00000 -cf firmware/0x00000.bin -ca 0x40000 -cf firmware/0x40000.bin -ca 0x12000 -cf webpages.espfs
minicom ttyUSB0

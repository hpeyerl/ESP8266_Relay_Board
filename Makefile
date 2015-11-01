include ../../esp-project.mk

all:
	$(MAKE) -C firmware

.PHONY: ESP8266_Relay_Board
ESP8266_Relay_Board:
	$(MAKE) -C ESP8266_Relay_Board

.PHONY: writeflash
writeflash:
	$(ESPTOOL) -v -eo firmware/build/httpd.out -bo firmware/firmware/0x00000.bin -bs .text -bs .data -bs .rodata -bc -ec -eo  firmware/build/httpd.out -es .irom0.text firmware/firmware/0x40000.bin -ec
	$(ESPTOOL) -v -cp /dev/ttyUSB0 -cd none -cb 115200 -ca 0x00000 -cf firmware/firmware/0x00000.bin -ca 0x40000 -cf firmware/firmware/0x40000.bin -ca 0x12000 -cf firmware/webpages.espfs
	minicom ttyUSB0

.PHONY: writeflash4
writeflash4:
	$(ESPTOOL) -v -eo firmware/build/httpd.out -bz 4M -bo firmware/firmware/0x00000.bin -bs .text -bs .data -bs .rodata -bc -ec -eo  firmware/build/httpd.out -es .irom0.text firmware/firmware/0x40000.bin -ec
	$(ESPTOOL) -v -cp /dev/ttyUSB0 -cd none -cb 115200 -ca 0x00000 -cf firmware/firmware/0x00000.bin -ca 0x40000 -cf firmware/firmware/0x40000.bin -ca 0x12000 -cf firmware/webpages.espfs
	minicom ttyUSB0

esptool.%:
	$(ESPTOOL) $*

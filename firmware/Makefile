# tnx to mamalala
# Changelog
# Changed the variables to include the header file directory
# Added global var for the XTENSA tool root
#
# This make file still needs some work.
#
#
# Output directors to store intermediate compiled files
# relative to the project directory
#
#   Local defines to override toolchain and sdk paths...
-include local.mk
#

BUILD_BASE	= build
FW_BASE		= firmware

# Base directory for the compiler
XTENSA_TOOLS_ROOT ?= /opt/Espressif/crosstool-NG/builds/xtensa-lx106-elf/bin

#Extra Tensilica includes from the ESS VM
SDK_EXTRA_INCLUDES ?= /opt/Espressif/ESP8266_SDK/include
SDK_EXTRA_LIBS ?= /opt/xtensa-lx106-elf/lib/gcc/xtensa-lx106-elf/4.8.2/

# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= /opt/Espressif/ESP8266_SDK

#Esptool.py path and port
ESPTOOL		?= esptool
ESPPORT		?= /dev/ttyUSB0
#ESPDELAY indicates seconds to wait between flashing the two binary images
ESPDELAY	?= 3
ESPBAUD		?= 115200

# name for the target project
TARGET		= httpd

# which modules (subdirectories) of the project to include in compiling
#MODULES		= driver user lwip/api lwip/app lwip/core lwip/core/ipv4 lwip/netif
MODULES		= driver user 
EXTRA_INCDIR	= include \
		  user/include \
		. \
		lib/heatshrink/ \
		$(SDK_EXTRA_INCLUDES)

CONFIG += -DCONFIG_MAX_RELAYS=2
CONFIG += -DCONFIG_MQTT
CONFIG += -DCONFIG_DHT22
# CONFIG += -DCONFIG_DS18B20
CONFIG += -DCONFIG_MAX31855
# CONFIG += -DCONFIG_I2C
# CONFIG += -DCONFIG_SI7020
# CONFIG += -DCONFIG_MLX91205
# CONFIG += -DCONFIG_GET_EXTERNAL_IP
# CONFIG += -DCONFIG_THERMOSTAT
CONFIG += -DCONFIG_WS2812B
CONFIG += -DCONFIG_TSL2561

# libraries used in this project, mainly provided by the SDK
LIBS		= c hal gcc phy pp net80211 wpa main lwip

# if GZIP_COMPRESSION is enabled then the static css, js, and html files will be compressed with gzip before added to the espfs image.
# This could speed up the downloading of these files, but might break compatibility with older web browsers not supporting gzip encoding
# because Accept-Encoding is simply ignored. Enable this option if you have large static files to serve (for e.g. JQuery, Twitter bootstrap)
GZIP_COMPRESSION = "yes"

# compiler flags using during compilation of source files
CFLAGS		= -Os -ggdb -std=c99 -Wpointer-arith -Wundef -Wall -Wl,-EL -fno-inline-functions \
		-nostdlib -mlongcalls -mtext-section-literals  -D__ets__ -DICACHE_FLASH \
		-Wno-address -Wno-unused-function -Wno-unused-but-set-variable $(CONFIG) -DSDK_VERSION=$(SDK_VERSION)

# linker flags used to generate the main object file
LDFLAGS		= -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static -L$(SDK_EXTRA_LIBS)
LDFLAGS += -Wl,-Map=out.map

# linker script used for the above linkier step
LD_SCRIPT	= eagle.app.v6.ld

# various paths from the SDK used in this project
SDK_LIBDIR	= lib
SDK_LDDIR	= ld
SDK_INCDIR	= include include/json

# we create two different files for uploading into the flash
# these are the names and options to generate them
FW_FILE_1	= 0x00000
FW_FILE_1_ARGS	= -bo $@ -bs .text -bs .data -bs .rodata -bc -ec
FW_FILE_2	= 0x40000
FW_FILE_2_ARGS	= -es .irom0.text $@ -ec
FW_FILE_3       = webpages.espfs

# select which tools to use as compiler, librarian and linker
CC		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
AR		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-ar
LD		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc



####
#### no user configurable options below here
####
SRC_DIR		:= $(MODULES)
BUILD_DIR	:= $(addprefix $(BUILD_BASE)/,$(MODULES))

SDK_LIBDIR	:= $(addprefix $(SDK_BASE)/,$(SDK_LIBDIR))
SDK_INCDIR	:= $(addprefix -I$(SDK_BASE)/,$(SDK_INCDIR))

SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
OBJ		:= $(patsubst %.c,$(BUILD_BASE)/%.o,$(SRC))
LIBS		:= $(addprefix -l,$(LIBS))
APP_AR		:= $(addprefix $(BUILD_BASE)/,$(TARGET)_app.a)
TARGET_OUT	:= $(addprefix $(BUILD_BASE)/,$(TARGET).out)

LD_SCRIPT	:= $(addprefix -T$(SDK_BASE)/$(SDK_LDDIR)/,$(LD_SCRIPT))

INCDIR	:= $(addprefix -I,$(SRC_DIR))
EXTRA_INCDIR	:= $(addprefix -I,$(EXTRA_INCDIR))
MODULE_INCDIR	:= $(addsuffix /include,$(INCDIR))

FW_FILE_1	:= $(addprefix $(FW_BASE)/,$(FW_FILE_1).bin)
FW_FILE_2	:= $(addprefix $(FW_BASE)/,$(FW_FILE_2).bin)

V ?= $(VERBOSE)
ifeq ("$(V)","1")
Q :=
vecho := @true
else
Q := @
vecho := @echo
endif

ifeq ($(GZIP_COMPRESSION),"yes")
CFLAGS		+= -DGZIP_COMPRESSION
endif

vpath %.c $(SRC_DIR)

define compile-objects
$1/%.o: %.c
	$(vecho) "CC $$<"
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS)  -c $$< -o $$@
endef

.PHONY: all checkdirs clean

all: checkdirs $(TARGET_OUT) $(FW_FILE_1) $(FW_FILE_2) $(FW_FILE_3)

$(FW_FILE_1): $(TARGET_OUT) firmware
	$(vecho) "FW $@"
	$(Q) $(ESPTOOL) -eo $(TARGET_OUT) $(FW_FILE_1_ARGS)

$(FW_FILE_2): $(TARGET_OUT) firmware
	$(vecho) "FW $@"
	$(Q) $(ESPTOOL) -eo $(TARGET_OUT) $(FW_FILE_2_ARGS)

$(FW_FILE_3): html/ html/config/ html/config/wifi/ html/control/ mkespfsimage/mkespfsimage
	$(vecho) "MKEFSIMAGE $@"
ifeq ($(GZIP_COMPRESSION),"yes")
	$(Q) rm -rf html_compressed;
		$(Q) cp -r html html_compressed;
		$(Q) cd html_compressed; find . -type f -regex ".*/.*\.\(html\|css\|js\)" -exec sh -c "gzip -n {}; mv {}.gz {}" \;; cd ..;	
		$(Q) cd html_compressed; find  | ../mkespfsimage/mkespfsimage > ../$(FW_FILE_3); cd ..;
else
		$(Q) cd html; find | ../mkespfsimage/mkespfsimage > ../$(FW_FILE_3); cd ..
endif
		$(Q) if [ $$(stat -c '%s' $(FW_FILE_3)) -gt $$(( 0x2E000 )) ]; then echo $(FW_FILE_3)" too big!"; false; fi

$(TARGET_OUT): $(APP_AR)
	$(vecho) "LD $@"
	$(Q) $(LD) -L$(SDK_LIBDIR) $(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@

$(APP_AR): $(OBJ)
	$(vecho) "AR $@"
	$(Q) $(AR) cru $@ $^

checkdirs: $(BUILD_DIR) $(FW_BASE)

$(BUILD_DIR):
	$(Q) mkdir -p $@

firmware:
	$(Q) mkdir -p $@

flash: $(FW_FILE_1) $(FW_FILE_2)
	$(Q) $(ESPTOOL) -cp $(ESPPORT) -cb $(ESPBAUD) -ca 0x00000 -cf firmware/0x00000.bin -v
	$(Q) [ $(ESPDELAY) -ne 0 ] && echo "Please put the ESP in bootloader mode..." || true
	$(Q) sleep $(ESPDELAY) || true
	$(Q) $(ESPTOOL) -cp $(ESPPORT) -cb $(ESPBAUD) -ca 0x40000 -cf firmware/0x40000.bin -v

mkespfsimage/mkespfsimage: mkespfsimage/
	make -C mkespfsimage

htmlflash: $(FW_FILE_3)
	if [ $$(stat -c '%s' $(FW_FILE_3)) -gt $$(( 0x2E000 )) ]; then echo $(FW_FILE_3)" too big!"; false; fi
	$(ESPTOOL) -cp $(ESPPORT) -cb $(ESPBAUD) -ca 0x12000 -cf $(FW_FILE_3) -v

clean:
	$(Q) rm -f $(APP_AR)
	$(Q) rm -f $(TARGET_OUT)
	$(Q) find $(BUILD_BASE) -type f | xargs rm -f

ifeq ($(GZIP_COMPRESSION),"yes") 
	$(Q) rm -rf html_compressed;
endif

	$(Q) rm -f $(FW_FILE_1)
	$(Q) rm -f $(FW_FILE_2)
	$(Q) rm -f $(FW_FILE_3)

	$(Q) rm -rf $(FW_BASE)

distclean: clean
	$(Q) make -C mkespfsimage clean

$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))

SDK_VERSION ?= 140
XTENSA_TOOLS_ROOT ?= /home/hpeyerl/phrobs/tools/xtensa-lx106-elf/bin/
SDK_BASE	?= /home/hpeyerl/phrobs/tools/esp_toolchain/esp_iot_sdk_v1.4.0
SDK_EXTRA_INCLUDES ?= $(SDK_BASE)/sdk/include $(SDK_BASE)/examples/driver_lib/include 
SDK_EXTRA_LIBS ?= $(SDK_BASE)/crosstool-NG/.build/xtensa-lx106-elf/buildtools/bin/xtensa-lx106-elf-gcc
#ESPTOOL		?= /home/hpeyerl/trunk/toolchains/esptool/esptool
ESPTOOL		?= /home/hpeyerl/phrobs/tools/esptool-ck/esptool

XTENSA_TOOLS_ROOT ?= /home/hpeyerl/trunk/toolchains/esp-open-sdk/xtensa-lx106-elf/bin/
SDK_EXTRA_INCLUDES ?= /home/hpeyerl/trunk/toolchains/esp-open-sdk/esp_iot_sdk_v1.2.0/sdk/include /home/hpeyerl/trunk/toolchains/esp-open-sdk/esp_iot_sdk_v1.2.0/examples/driver_lib/include 
SDK_EXTRA_LIBS ?= /home/hpeyerl/trunk/toolchains/esp-open-sdk/crosstool-NG/.build/xtensa-lx106-elf/buildtools/bin/xtensa-lx106-elf-gcc
SDK_BASE	?= /home/hpeyerl/trunk/toolchains/esp-open-sdk/esp_iot_sdk_v1.2.0
#ESPTOOL		?= /home/hpeyerl/trunk/toolchains/esptool/esptool
ESPTOOL		?= /home/hpeyerl/phrobs/tools/esptool-ck/esptool

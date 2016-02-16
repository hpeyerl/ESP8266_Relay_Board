SDK_VERSION ?= 140
XTENSA_TOOLS_ROOT ?= /home/hpeyerl/trunk/toolchains/esp-open-sdk/xtensa-lx106-elf/bin/
SDK_EXTRA_INCLUDES ?= /home/hpeyerl/phrobs/tools/esp_toolchain/esp_iot_sdk_v1.4.0/sdk/include /home/hpeyerl/phrobs/tools/esp_toolchain/esp_iot_sdk_v1.4.0/examples/driver_lib/include 
SDK_EXTRA_LIBS ?= /home/hpeyerl/trunk/toolchains/esp-open-sdk/crosstool-NG/.build/xtensa-lx106-elf/buildtools/bin/xtensa-lx106-elf-gcc
SDK_BASE	?= /home/hpeyerl/phrobs/tools/esp_toolchain/esp_iot_sdk_v1.4.0
ESPTOOL		?= /home/hpeyerl/trunk/toolchains/esptool/esptool

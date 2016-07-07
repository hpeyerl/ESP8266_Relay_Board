-include ../../../esp-project.mk
SDK_VERSION ?= 154
XTENSA_TOOLS_ROOT ?= $(TOOLCHAIN_DIR)
SDK_BASE	?= $(TOOLS_DIR)/ESP8266_NONOS_SDK_V1.5.4_16_05_20
SDK_EXTRA_INCLUDES ?= $(SDK_BASE)/sdk/include $(SDK_BASE)/examples/driver_lib/include 
SDK_EXTRA_LIBS ?= $(SDK_BASE)/crosstool-NG/.build/xtensa-lx106-elf/buildtools/bin/xtensa-lx106-elf-gcc

FAMILY ?= rp2040

PICO_SDK_PATH ?= lib/picosdk
TINYUSB_PATH ?= lib/tinyusb

PICO_SCK_FILES = $(PICO_SDK_PATH)/CMakeLists.txt
TINYUSB_FILES = $(TINYUSB_PATH)/hw/bsp/family_support.cmake

CMAKE_FLAGS = -DFAMILY=$(FAMILY) -DPICO_SDK_PATH=$(PICO_SDK_PATH)
CMAKE_FLAGS_LEFT = $(CMAKE_FLAGS) -DSPLIT_SIDE=LEFT $(CMAKE_FLAGS_LEFT_EXTRA)
CMAKE_FLAGS_RIGHT = $(CMAKE_FLAGS) -DSPLIT_SIDE=RIGHT $(CMAKE_FLAGS_RIGHT_EXTRA)

all: left right

clean:
	rm -rf .build

left: | $(PICO_SDK_FILES) $(TINYUSB_FILES) .build/left
	cmake -B .build/left $(CMAKE_FLAGS_LEFT)
	make -C .build/left

right: | $(PICO_SDK_FILES $(TINYUSB_FILES) .build/right
	cmake -B .build/right $(CMAKE_FLAGS_RIGHT)
	make -C .build/right

$(PICO_SDK_FILES):
	git submodule update --init lib/picosdk

$(TINYUSB_FILES):
	git submodule update --init lib/tinyusb
	git -C lib/tinyusb apply ../../extra/tinyusb.diff

.build/left .build/right:
	mkdir -p $@

flash_left:
	picotool load .build/left/sxkbd.uf2

flash_right:
	picotool load .build/right/sxkbd.uf2

.PHONY: all clean left right flash_left flash_right

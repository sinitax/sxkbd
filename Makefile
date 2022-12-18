FAMILY ?= rp2040
PICO_SDK_PATH ?= lib/picosdk

CMAKE_FLAGS = -DFAMILY=$(FAMILY) -DPICO_SDK_PATH=$(PICO_SDK_PATH)
CMAKE_FLAGS_LEFT = $(CMAKE_FLAGS) -DSPLIT_SIDE=LEFT $(CMAKE_FLAGS_LEFT_EXTRA)
CMAKE_FLAGS_RIGHT = $(CMAKE_FLAGS) -DSPLIT_SIDE=RIGHT $(CMAKE_FLAGS_RIGHT_EXTRA)

all: left right

clean:
	rm -rf .build

left: | $(PICO_SDK_PATH) .build/left
	cmake -B .build/left $(CMAKE_FLAGS_LEFT)
	make -C .build/left

right: | $(PICO_SDK_PATH) .build/right
	cmake -B .build/right $(CMAKE_FLAGS_RIGHT)
	make -C .build/right

lib/picosdk:
	git submodule update --init lib/picosdk

.build/left:
	mkdir -p $@

.build/right:
	mkdir -p $@

flash_left:
	picotool load .build/left/sxkbd.uf2

flash_right:
	picotool load .build/right/sxkbd.uf2

.PHONY: all clean left right upload

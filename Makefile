FAMILY ?= rp2040
PICO_SDK_PATH ?= lib/picosdk

all: cmake

clean:
	rm -rf .build

cmake: | $(PICO_SDK_PATH) .build
	cmake -B .build -DFAMILY=$(FAMILY) -DPICO_SDK_PATH=$(PICO_SDK_PATH)
	make -C .build

lib/picosdk:
	git submodule update --init lib/picosdk

.build:
	mkdir $@

flash:
	picotool load .build/sxkbd.uf2

.PHONY: all clean cmake upload

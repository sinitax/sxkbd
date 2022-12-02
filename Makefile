FAMILY ?= rp2040
PICO_SDK_PATH ?= lib/picosdk

all: left

clean:
	rm -rf .build

left: | $(PICO_SDK_PATH) .build/left
	cmake -B .build/left -DFAMILY=$(FAMILY) -DPICO_SDK_PATH=$(PICO_SDK_PATH) \
		-DSPLIT_SIDE=LEFT -DSPLIT_ROLE=SLAVE
	make -C .build/left

right: | $(PICO_SDK_PATH) .build/right
	cmake -B .build/right -DFAMILY=$(FAMILY) -DPICO_SDK_PATH=$(PICO_SDK_PATH) \
		-DSPLIT_SIDE=RIGHT -DSPLIT_ROLE=MASTER
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

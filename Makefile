FAMILY ?= rp2040

SIDE ?= left
SPLIT_SIDE = $(shell echo "$(SIDE)" | tr a-z A-Z)

PICO_SDK_PATH ?= lib/picosdk
TINYUSB_PATH ?= lib/tinyusb

PICO_SDK_FILES = $(PICO_SDK_PATH)/CMakeLists.txt
TINYUSB_FILES = $(TINYUSB_PATH)/hw/bsp/family_support.cmake

CMAKE_FLAGS = -DFAMILY=$(FAMILY) -DPICO_SDK_PATH=$(PICO_SDK_PATH)
CMAKE_FLAGS += -DSPLIT_SIDE=$(SPLIT_SIDE) $(CMAKE_FLAGS_LEFT_EXTRA)
ifdef ROLE
SPLIT_ROLE = $(shell echo "$(ROLE)" | tr a-z A-Z)
CMAKE_FLAGS += -DSPLIT_ROLE=$(SPLIT_ROLE)
endif

all: build flash

clean:
	rm -rf .build

build: | $(PICO_SDK_FILES) $(TINYUSB_FILES) .build/$(SIDE)
	cmake -B .build/$(SIDE) $(CMAKE_FLAGS)
	make -C .build/$(SIDE)

flash:
	picotool load .build/$(SIDE)/sxkbd.uf2

$(PICO_SDK_FILES):
	git submodule update --init lib/picosdk

$(TINYUSB_FILES):
	git submodule update --init lib/tinyusb
	git -C lib/tinyusb apply ../../extra/tinyusb.diff

.build/$(SIDE):
	mkdir -p $@

.PHONY: all clean build flash

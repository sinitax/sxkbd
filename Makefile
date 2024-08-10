FAMILY ?= rp2040

SIDE ?= left

PICO_SDK_PATH ?= lib/picosdk
TINYUSB_PATH ?= lib/tinyusb

PICO_SDK_FILES = $(PICO_SDK_PATH)/CMakeLists.txt
TINYUSB_FILES = $(TINYUSB_PATH)/hw/bsp/family_support.cmake

CMAKE_FLAGS = -DFAMILY=$(FAMILY) -DPICO_SDK_PATH=$(PICO_SDK_PATH)
CMAKE_C_FLAGS = -DSPLIT_SIDE=$(shell echo "$(SIDE)" | tr a-z A-Z)
ifdef ROLE
CMAKE_C_FLAGS += -DSPLIT_ROLE=$(shell echo "$(ROLE)" | tr a-z A-Z)
endif
ifneq ($(GPIO_MOD),0)
CMAKE_C_FLAGS += -DBAD_GPIO_MITIGATION=1
endif

all: build flash

clean:
	rm -rf .build

build: | $(PICO_SDK_FILES) $(TINYUSB_FILES) .build/$(SIDE)
	cmake -B .build/$(SIDE) $(CMAKE_FLAGS) -DCMAKE_C_EXTRA_FLAGS="$(CMAKE_C_FLAGS)"
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

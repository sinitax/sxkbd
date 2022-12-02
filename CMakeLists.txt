cmake_minimum_required(VERSION 3.5)

include(${CMAKE_CURRENT_SOURCE_DIR}/lib/tinyusb/hw/bsp/family_support.cmake)

family_get_project_name(PROJECT ${CMAKE_CURRENT_LIST_DIR})

project(${PROJECT})

family_initialize_project(${PROJECT} ${CMAKE_CURRENT_LIST_DIR})

add_executable(${PROJECT})

pico_generate_pio_header(${PROJECT} ${CMAKE_CURRENT_SOURCE_DIR}/src/ws2812.pio)
pico_generate_pio_header(${PROJECT} ${CMAKE_CURRENT_SOURCE_DIR}/src/uart_tx.pio)
pico_generate_pio_header(${PROJECT} ${CMAKE_CURRENT_SOURCE_DIR}/src/uart_rx.pio)

target_sources(${PROJECT} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/util.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/hid.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/split.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/ws2812.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/led.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/matrix.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/keymap.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/usb_stdio.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/usb_descriptors.c
)

target_include_directories(${PROJECT} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${PICO_SDK_PATH}/src/rp2_common/hardware_gpio/include
	${PICO_SDK_PATH}/src/rp2_common/hardware_pio/include
	${PICO_SDK_PATH}/src/rp2_common/hardware_timer/include
	${PICO_SDK_PATH}/src/rp2_common/hardware_uart/include
	${PICO_SDK_PATH}/src/rp2_common/hardware_clocks/include
)

target_link_libraries(${PROJECT} PRIVATE
	pico_stdlib
	pico_stdio
	pico_bootsel_via_double_reset
	hardware_pio
	hardware_gpio
	tinyusb_device
)

target_compile_options(pico_stdio INTERFACE
	-Wno-shadow # issue in picosdk/src/rp2_common/hardware_flash/flash.c
	-Wno-unused-parameter
)

target_compile_options(${PROJECT} PRIVATE
	"-DSPLIT_SIDE=${SPLIT_LEFT}"
	"-DSPLIT_ROLE=${SPLIT_ROLE}"
)

family_configure_target(${PROJECT})
family_add_default_example_warnings(${PROJECT})
suppress_tinyusb_warnings()

#pico_enable_stdio_usb(${PROJECT} 1)
pico_enable_stdio_uart(${PROJECT} 0)

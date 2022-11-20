cmake_minimum_required(VERSION 3.5)

include(${CMAKE_CURRENT_SOURCE_DIR}/lib/tinyusb/hw/bsp/family_support.cmake)

family_get_project_name(PROJECT ${CMAKE_CURRENT_LIST_DIR})

project(${PROJECT})

family_initialize_project(${PROJECT} ${CMAKE_CURRENT_LIST_DIR})

add_executable(${PROJECT})

target_sources(${PROJECT} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/neopix.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/usb_descriptors.c
)

target_include_directories(${PROJECT} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${PICO_SDK_PATH}/src/rp2_common/hardware_gpio/include
	${PICO_SDK_PATH}/src/rp2_common/hardware_pio/include
)

target_link_libraries(${PROJECT} PRIVATE
	pico_stdlib
	hardware_pio
	hardware_gpio
)

# Configure compilation flags and libraries for the example... see the corresponding function
# in hw/bsp/FAMILY/family.cmake for details.
family_configure_device_example(${PROJECT})

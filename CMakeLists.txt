# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
list(APPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(stm32h750)

target_sources(app PRIVATE
	src/app_main.c
	src/hal_gpio.c
	src/hal_uart.c
	src/hal_rtc.c
	src/hal_adc.c
	src/hal_pwm.c
	src/qspi_flash.c
	src/fs_nvs.c
	src/sd_disk.c
	src/bsp_sd.c
	src/cpu_temp.c
)

# add_compile_definitions(HEADSET=1)

# Application sources
# add_subdirectory(src/audio)

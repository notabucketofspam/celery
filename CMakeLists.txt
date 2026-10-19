cmake_minimum_required(VERSION 3.19)

set(TARGET_BOARD "PI" CACHE STRING "Select target board: PI or AVR")

project(TrackerWorkspace LANGUAGES C CXX ASM)

if(TARGET_BOARD STREQUAL "PI")
    message(STATUS "[Workspace] Configuring Raspberry Pi 4 OpenCV Tracker...")
    add_subdirectory(pi-tracker)
elseif(TARGET_BOARD STREQUAL "AVR")
    message(STATUS "[Workspace] Configuring RAMPS ATmega2560 Firmware...")
    add_subdirectory(ramps-firmware)
else()
    message(FATAL_ERROR "Unknown TARGET_BOARD: ${TARGET_BOARD}")
endif()

cmake_minimum_required(VERSION 3.19)
project(MegaStepper LANGUAGES C CXX ASM)

set(MCU "atmega2560")
set(F_CPU "16000000UL")

# you gotta set this to be the actual COM port for the Arduino Mega
set(UPLOAD_PORT "COM7")

# Standard Arduino AVR compiler flags
add_compile_options(
    -mmcu=${MCU}
    -DF_CPU=${F_CPU}
    -DARDUINO=10819
    -DARDUINO_AVR_MEGA2560
    -DARDUINO_ARCH_AVR
    -Os
    -Wall
    -ffunction-sections
    -fdata-sections
    $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++11>
    $<$<COMPILE_LANGUAGE:CXX>:-fpermissive>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>
    $<$<COMPILE_LANGUAGE:C>:-std=gnu11>
    $<$<COMPILE_LANGUAGE:ASM>:-x>
    $<$<COMPILE_LANGUAGE:ASM>:assembler-with-cpp>
)

add_link_options(
    -mmcu=${MCU}
    -Os
    -Wl,--gc-sections
)

# ---------------------------------------------------------
# 1. Arduino Core (OBJECT library preserves ISRs and main())
# ---------------------------------------------------------
file(GLOB ARDUINO_CORE_SRC
    "lib/arduino_core/cores/arduino/*.c"
    "lib/arduino_core/cores/arduino/*.cpp"
    "lib/arduino_core/cores/arduino/*.S"
)

add_library(arduino_core OBJECT ${ARDUINO_CORE_SRC})
target_include_directories(arduino_core PUBLIC
    "lib/arduino_core/cores/arduino"
    "lib/arduino_core/variants/mega"
)
target_compile_options(arduino_core PRIVATE -w)

# ---------------------------------------------------------
# 2. AccelStepper Library
# ---------------------------------------------------------
file(GLOB ACCELSTEPPER_SRC "lib/AccelStepper/*.cpp")
add_library(accelstepper STATIC ${ACCELSTEPPER_SRC})
target_include_directories(accelstepper PUBLIC "lib/AccelStepper")
target_link_libraries(accelstepper PUBLIC arduino_core)

# ---------------------------------------------------------
# 3. Main Firmware Executable (.elf) and Firmware (.hex)
# ---------------------------------------------------------
add_executable(${PROJECT_NAME}.elf src/main.cpp)
target_link_libraries(${PROJECT_NAME}.elf PRIVATE arduino_core accelstepper m)

# Automatically convert .elf to .hex and print flash usage on every build
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND "${AVR_OBJCOPY}" -O ihex -R .eeprom $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.hex
    COMMAND "${AVR_SIZE}" -B $<TARGET_FILE:${PROJECT_NAME}.elf>
    COMMENT "Generating ${PROJECT_NAME}.hex and checking memory usage..."
)

# ---------------------------------------------------------
# 4. Flash Target (avrdude)
# ---------------------------------------------------------
add_custom_target(upload
    COMMAND avrdude -p ${MCU} -c wiring -P ${UPLOAD_PORT} -b 115200 -D -U flash:w:${PROJECT_NAME}.hex:i
    DEPENDS ${PROJECT_NAME}.elf
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Flashing ${PROJECT_NAME}.hex to ATmega2560 on ${UPLOAD_PORT}..."
)

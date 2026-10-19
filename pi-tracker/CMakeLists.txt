cmake_minimum_required(VERSION 3.16)
project(DroneTracker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# OpenCV 5.x installs its CMake config into opencv5/
set(OpenCV_DIR "/usr/local/lib/cmake/opencv5")
find_package(OpenCV REQUIRED COMPONENTS core imgproc video videoio tracking highgui)

# Explicitly list source files from the src directory
set(SOURCES
    src/main.cpp
    src/PiCameraStream.cpp
    src/UartController.cpp
    src/PDController.cpp
    src/SkyDetector.cpp
)

add_executable(DroneTracker ${SOURCES})

# Expose both the local include directory and OpenCV headers
target_include_directories(DroneTracker PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(DroneTracker PRIVATE 
    ${OpenCV_LIBS} 
    pthread
)

# Enable Cortex-A72 hardware optimizations
target_compile_options(DroneTracker PRIVATE -O3 -mcpu=cortex-a72)

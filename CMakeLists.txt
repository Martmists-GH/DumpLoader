cmake_minimum_required(VERSION 3.12)
project(dumploader C CXX)

include_directories(/opt/devkitpro/devkitARM/include)
include_directories(/opt/devkitpro/libctru/include)
include_directories(/opt/devkitpro/devkitARM/arm-none-eabi/include)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_COMPILER /opt/devkitpro/devkitARM/bin/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER /opt/devkitpro/devkitARM/bin/arm-none-eabi-g++)

add_executable(
        dumploader
        source/main.cpp
)

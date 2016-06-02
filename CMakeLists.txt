cmake_minimum_required(VERSION 3.5)
project(PostESP)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    doc/esp-12-pinout.png
    doc/nodemcu_pinout.png
    lib/readme.txt
    src/main.cpp
    src/user_config.h
    platformio.ini
    README.md)

add_executable(PostESP ${SOURCE_FILES})
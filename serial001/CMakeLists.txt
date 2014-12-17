cmake_minimum_required(VERSION 2.8.4)
project(serial001)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

set(SOURCE_FILES
    serial.c
    parameter.h
    parameter.c)

add_executable(serial ${SOURCE_FILES})
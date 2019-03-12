cmake_minimum_required(VERSION 3.3)
project(sysopy C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

set(SOURCE_FILES main.c)

add_executable(cw02 ${SOURCE_FILES})
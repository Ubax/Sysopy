cmake_minimum_required(VERSION 2.8)
project(sysopy C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_library(libMonitor monitor.c)
add_executable(cw03 main.c)
add_executable(tester tester.c)

target_link_libraries(cw03 libMonitor)

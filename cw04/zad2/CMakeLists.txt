cmake_minimum_required(VERSION 2.8)
project(sysopy C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_library(libMonitor monitor.c)
add_library(libFileAnalyzer fileAnalyzer.c)
add_executable(cw04 main.c)

target_link_libraries(cw04 libMonitor)
target_link_libraries(cw04 libFileAnalyzer)



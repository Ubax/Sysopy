cmake_minimum_required(VERSION 2.8)
project(sysopy C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_library(argsProc argsProcessor.c)

add_executable(sender sender.c)
add_executable(catcher catcher.c)
target_link_libraries(sender argsProc)


cmake_minimum_required(VERSION 2.8)
project(sysopy C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_library(sysopy sysopy.c)
add_library(load load.c)

add_executable(truck truck.c)
add_executable(loader loader.c)
add_executable(init init.c)

target_link_libraries(truck sysopy)
target_link_libraries(loader sysopy)
target_link_libraries(init sysopy)
target_link_libraries(truck load)
target_link_libraries(loader load)
target_link_libraries(truck -lpthread)
target_link_libraries(truck -lrt)
target_link_libraries(loader -lpthread)
target_link_libraries(loader -lrt)

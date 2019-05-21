cmake_minimum_required(VERSION 2.8)
project(sysopy C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_library(sysopy sysopy.c)
add_library(queue queue.c)

add_executable(main main.c)

target_link_libraries(main sysopy)
target_link_libraries(main queue)
target_link_libraries(main -lpthread)
target_link_libraries(main -lrt)

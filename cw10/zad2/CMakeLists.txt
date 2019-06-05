cmake_minimum_required(VERSION 2.8)
project(sysopy C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_library(sysopy sysopy.c)

add_executable(server server.c)
add_executable(client client.c)

target_link_libraries(server sysopy)
target_link_libraries(server -lpthread)
target_link_libraries(server -lrt)

target_link_libraries(client sysopy)
target_link_libraries(client -lpthread)
target_link_libraries(client -lrt)

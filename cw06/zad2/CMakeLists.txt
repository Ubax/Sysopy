cmake_minimum_required(VERSION 2.8)
project(sysopy C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_library(libArgsProcessor argsProcessor.c)

add_executable(server server.c)

add_executable(client client.c)
target_link_libraries(client libArgsProcessor)
find_library(LIBRT rt)
if(LIBRT)
    target_link_libraries(server ${LIBRT})
    target_link_libraries(client ${LIBRT})
endif()


cmake_minimum_required(VERSION 2.8)
project(sysopy C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_library(sysopy sysopy.c)

add_executable(truck truck.c)
add_executable(loader loader.c)

target_link_libraries(truck sysopy)
target_link_libraries(loader sysopy)
# find_library(LIBRT rt)
# if(LIBRT)
#     target_link_libraries(server ${LIBRT})
#     target_link_libraries(client ${LIBRT})
# endif()

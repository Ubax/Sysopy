cmake_minimum_required(VERSION 3.3)
project(sysopy C)

set(CMAKE_C_STANDARD 99)
set(GCC_COVERAGE_COMPILE_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS "-Wall -pedantic -Wextra")

set(LIBRARY_SRC library.c)
set(SOURCE_FILES main.c)

add_library(libCw01 ${LIBRARY_SRC})
add_executable(cw01 ${SOURCE_FILES})

target_link_libraries(cw01 libCw01)
target_compile_options(cw01 PRIVATE "-Wall")


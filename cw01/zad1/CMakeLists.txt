cmake_minimum_required(VERSION 3.3)
project(sysopy C)

set(CMAKE_C_STANDARD 99)
set(GCC_COVERAGE_COMPILE_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS "-Wall -pedantic -Wextra")

set(LIBRARY_SRC library.c)

add_library(libZad1 SHARED ${LIBRARY_SRC})
add_library(libZad1 STATIC ${LIBRARY_SRC})

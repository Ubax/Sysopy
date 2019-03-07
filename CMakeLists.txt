cmake_minimum_required(VERSION 2.8)
project(sysopy C)

set(CMAKE_C_STANDARD 99)
set(GCC_COVERAGE_COMPILE_FLAGS "-Wall")

add_library(zad1lib zad1lib/library.c)

add_executable(sysopy main.c)
target_link_libraries(sysopy zad1lib)


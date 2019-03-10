cmake_minimum_required(VERSION 3.3)
project(sysopy C)

set(CMAKE_C_STANDARD 99)
set(GCC_COVERAGE_COMPILE_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS "-Wall -pedantic -Wextra")

set(LIBRARY_SRC library.c)
set(SOURCE_FILES main.c)

add_library(libZad1 SHARED ${LIBRARY_SRC})
add_executable(zad1 ${SOURCE_FILES})

target_link_libraries(zad1 zad1lib)
target_compile_options(zad1 PRIVATE "-Wall")


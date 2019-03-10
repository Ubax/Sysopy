cmake_minimum_required(VERSION 3.3)
project(sysopy C)

set(CMAKE_C_STANDARD 99)
set(GCC_COVERAGE_COMPILE_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS "-Wall -pedantic -Wextra")

set(LIBRARY_SRC library.c)
set(SOURCE_FILES main.c)

add_library(libZad1_static STATIC ${LIBRARY_SRC})
add_library(libZad1_shared SHARED ${LIBRARY_SRC})

add_executable(zad1_static ${SOURCE_FILES})
add_executable(zad1_shared ${SOURCE_FILES})
add_executable(zad1_dynamic ${SOURCE_FILES})

target_link_libraries(zad1_static libZad1_static)
target_link_libraries(zad1_shared libZad1_shared)
target_link_libraries(zad1_dynamic libZad1_shared)

target_compile_options(zad1_static PRIVATE "-Wall")
target_compile_options(zad1_shared PRIVATE "-Wall")
target_compile_options(zad1_dynamic PRIVATE "-ldl -D DLL -Wall")

execute_process(COMMAND "./zad1_static > resultS.txt")
execute_process(COMMAND "./zad1_shared > resultSh.txt")
execute_process(COMMAND "./zad1_dynamic > resultD.txt")
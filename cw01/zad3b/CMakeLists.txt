cmake_minimum_required(VERSION 3.3)
project(sysopy C)

set(CMAKE_C_STANDARD 99)
set(GCC_COVERAGE_COMPILE_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS "-Wall -pedantic -Wextra")

set(LIBRARY_SRC library.c)
set(SOURCE_FILES main.c)

add_library(libCw01_static STATIC ${LIBRARY_SRC})
add_library(libCw01_shared SHARED ${LIBRARY_SRC})

add_executable(cw01_static ${SOURCE_FILES})
add_executable(cw01_shared ${SOURCE_FILES})
add_executable(cw01_dynamic ${SOURCE_FILES})

target_link_libraries(cw01_static libCw01_static)
target_link_libraries(cw01_shared libCw01_shared)
target_link_libraries(cw01_dynamic libCw01_shared)

target_compile_options(cw01_static PRIVATE "-Wall")
target_compile_options(cw01_shared PRIVATE "-Wall")
target_compile_options(cw01_dynamic PRIVATE "-ldl -D DLL -Wall")

execute_process(COMMAND "./cw01_static > resultS.txt")
execute_process(COMMAND "./cw01_shared > resultSh.txt")
execute_process(COMMAND "./cw01_dynamic > resultD.txt")
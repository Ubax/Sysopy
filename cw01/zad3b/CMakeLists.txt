cmake_minimum_required(VERSION 3.3)
project(sysopy C)

option(OPTIMIZATION_TYPE "Type of optimalization" O0)
SET(OPTIMIZATION_FLAG "-O0")

if(OPTIMIZATION_TYPE STREQUAL Os)
    message(STATUS "Mode set to Os")
    SET(OPTIMIZATION_FLAG "-Os")
endif(OPTIMIZATION_TYPE STREQUAL Os)

if(OPTIMIZATION_TYPE STREQUAL O3)
    message(STATUS "Mode set to O3")
    SET(OPTIMIZATION_FLAG "-O3")
endif(OPTIMIZATION_TYPE STREQUAL O3)

if(OPTIMIZATION_TYPE STREQUAL O2)
    message(STATUS "Mode set to O2")
    SET(OPTIMIZATION_FLAG "-O2")
endif(OPTIMIZATION_TYPE STREQUAL O2)


if(OPTIMIZATION_TYPE STREQUAL O1)
    message(STATUS "Mode set to O1")
    SET(OPTIMIZATION_FLAG "-O1")
endif(OPTIMIZATION_TYPE STREQUAL O1)


set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall ${OPTIMIZATION_FLAG}")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Wall ${OPTIMIZATION_FLAG}")

set(LIBRARY_SRC library.c)
set(SOURCE_FILES main.c)
set(RESULTS_FILE results3b.txt)

set(SMALL_TEST 100 search_directory . \"*.txt\" tmp.txt add_to_table tmp.txt remove_block 0 add_to_table tmp.txt
        remove_block 0)

set(MEDIUM_TEST 100 search_directory ~ \"*.txt\" tmp.txt add_to_table tmp.txt remove_block 0 add_to_table tmp.txt
        remove_block 0
        )

set(BIG_TEST 100 search_directory / \"*.txt\" tmp.txt add_to_table tmp.txt remove_block 0 add_to_table tmp.txt
        remove_block 0 )

add_library(libCw01_static STATIC ${LIBRARY_SRC})
add_library(libCw01_shared SHARED ${LIBRARY_SRC})

add_executable(cw01_static ${SOURCE_FILES})
add_executable(cw01_shared ${SOURCE_FILES})
add_executable(cw01_dynamic ${SOURCE_FILES})

target_link_libraries(cw01_static libCw01_static)
target_link_libraries(cw01_shared libCw01_shared)
target_link_libraries(cw01_dynamic libCw01_shared)
target_link_libraries(cw01_dynamic dl)

target_compile_options(cw01_static PRIVATE "-Wall")
target_compile_options(cw01_shared PRIVATE "-Wall")
target_compile_options(cw01_dynamic PRIVATE "-ldl -D DLL -Wall")

add_custom_command(
        TARGET cw01_static
        POST_BUILD COMMAND  echo "---- STATIC SMALL ----" >> ${RESULTS_FILE}  & ./cw01_static ${SMALL_TEST}
)
add_custom_command(
        TARGET cw01_static
        POST_BUILD COMMAND  echo "---- STATIC MEDIUM ----" >> ${RESULTS_FILE}  & ./cw01_static ${MEDIUM_TEST}
)
add_custom_command(
        TARGET cw01_static
        POST_BUILD COMMAND  echo "---- STATIC BIG ----" >> ${RESULTS_FILE}  & ./cw01_static ${BIG_TEST}
)

add_custom_command(
        TARGET cw01_shared
        POST_BUILD COMMAND echo "---- SHARED SMALL ----" >> ${RESULTS_FILE}  & ./cw01_shared ${SMALL_TEST}
)
add_custom_command(
        TARGET cw01_shared
        POST_BUILD COMMAND echo "---- SHARED MEDIUM ----" >> ${RESULTS_FILE}  & ./cw01_shared ${MEDIUM_TEST}
)
add_custom_command(
        TARGET cw01_shared
        POST_BUILD COMMAND echo "---- SHARED BIG ----" >> ${RESULTS_FILE}  & ./cw01_shared ${BIG_TEST}
)

add_custom_command(
        TARGET cw01_dynamic
        POST_BUILD COMMAND echo "---- DYNAMIC SMALL ----" >> ${RESULTS_FILE}  & ./cw01_dynamic ${SMALL_TEST}
)
add_custom_command(
        TARGET cw01_dynamic
        POST_BUILD COMMAND echo "---- DYNAMIC MEDIUM ----" >> ${RESULTS_FILE}  & ./cw01_dynamic ${MEDIUM_TEST}
)
add_custom_command(
        TARGET cw01_dynamic
        POST_BUILD COMMAND echo "---- DYNAMIC BIG ----" >> ${RESULTS_FILE}  & ./cw01_dynamic ${BIG_TEST}
)




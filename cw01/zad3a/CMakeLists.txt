cmake_minimum_required(VERSION 3.3)
project(sysopy C)

set(CMAKE_C_STANDARD 99)
set(GCC_COVERAGE_COMPILE_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS "-Wall -pedantic -Wextra")

set(LIBRARY_SRC library.c)
set(SOURCE_FILES main.c)
set(RESULTS_FILE results3a.txt)

set(SMALL_TEST 100 search_directory . "*.txt" tmp.txt add_to_table tmp.txt remove_block 0 add_to_table tmp.txt
        remove_block 0 add_to_table tmp.txt remove_block 0 add_to_table tmp.txt remove_block 0
        add_to_table tmp.txt add_to_table tmp.txt add_to_table tmp.txt add_to_table tmp.txt
        remove_block 0 remove_block 1 remove_block 2 remove_block 3
        add_to_table tmp.txt add_to_table tmp.txt add_to_table tmp.txt add_to_table tmp.txt
        add_to_table tmp.txt add_to_table tmp.txt add_to_table tmp.txt add_to_table tmp.txt
        remove_block 0 remove_block 1 remove_block 2 remove_block 3
        remove_block 4 remove_block 5 remove_block 6 remove_block 7)

set(MEDIUM_TEST 100 search_directory ~ "*.txt" tmp.txt add_to_table tmp.txt remove_block 0 add_to_table tmp.txt
        remove_block 0 add_to_table tmp.txt remove_block 0 add_to_table tmp.txt remove_block 0
        add_to_table tmp.txt add_to_table tmp.txt add_to_table tmp.txt add_to_table tmp.txt
        remove_block 0 remove_block 1 remove_block 2 remove_block 3
        add_to_table tmp.txt add_to_table tmp.txt add_to_table tmp.txt add_to_table tmp.txt
        add_to_table tmp.txt add_to_table tmp.txt add_to_table tmp.txt add_to_table tmp.txt
        remove_block 0 remove_block 1 remove_block 2 remove_block 3
        remove_block 4 remove_block 5 remove_block 6 remove_block 7)

set(BIG_TEST 100 search_directory / "*.txt" tmp.txt add_to_table tmp.txt remove_block 0 add_to_table tmp.txt
        remove_block 0 add_to_table tmp.txt remove_block 0 add_to_table tmp.txt remove_block 0
        add_to_table tmp.txt add_to_table tmp.txt add_to_table tmp.txt add_to_table tmp.txt
        remove_block 0 remove_block 1 remove_block 2 remove_block 3
        add_to_table tmp.txt add_to_table tmp.txt add_to_table tmp.txt add_to_table tmp.txt
        add_to_table tmp.txt add_to_table tmp.txt add_to_table tmp.txt add_to_table tmp.txt
        remove_block 0 remove_block 1 remove_block 2 remove_block 3
        remove_block 4 remove_block 5 remove_block 6 remove_block 7)

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

add_custom_command(
        TARGET cw01_static
        POST_BUILD COMMAND echo "---- STATIC SMALL ----" >> ${RESULTS_FILE} >> &./cw01_static ${SMALL_TEST}
)
add_custom_command(
        TARGET cw01_static
        POST_BUILD COMMAND echo "---- STATIC MEDIUM ----" >> ${RESULTS_FILE} >> &./cw01_static ${MEDIUM_TEST}
)
add_custom_command(
        TARGET cw01_static
        POST_BUILD COMMAND echo "---- STATIC BIG ----" >> ${RESULTS_FILE} >> &./cw01_static ${BIG_TEST}
)

add_custom_command(
        TARGET cw01_shared
        POST_BUILD COMMAND echo "---- SHARED SMALL ----" >> ${RESULTS_FILE} >> &./cw01_shared ${SMALL_TEST}
)
add_custom_command(
        TARGET cw01_shared
        POST_BUILD COMMAND echo "---- SHARED MEDIUM ----" >> ${RESULTS_FILE} >> &./cw01_shared ${MEDIUM_TEST}
)
add_custom_command(
        TARGET cw01_shared
        POST_BUILD COMMAND echo "---- SHARED BIG ----" >> ${RESULTS_FILE} >> &./cw01_shared ${BIG_TEST}
)

add_custom_command(
        TARGET cw01_dynamic
        POST_BUILD COMMAND echo "---- DYNAMIC SMALL ----" >> ${RESULTS_FILE} >> &./cw01_dynamic ${SMALL_TEST}
)
add_custom_command(
        TARGET cw01_dynamic
        POST_BUILD COMMAND echo "---- DYNAMIC MEDIUM ----" >> ${RESULTS_FILE} >> &./cw01_dynamic ${MEDIUM_TEST}
)
add_custom_command(
        TARGET cw01_dynamic
        POST_BUILD COMMAND echo "---- DYNAMIC BIG ----" >> ${RESULTS_FILE} >> &./cw01_dynamic ${BIG_TEST}
)
cmake_minimum_required(VERSION 3.13)
project(zad1lib C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS -Wall)

set(libsrc library.c)

add_library(zad1libShared SHARED ${libsrc})
add_library(zad1libStatic STATIC ${libsrc})
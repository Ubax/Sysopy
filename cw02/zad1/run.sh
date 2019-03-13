#!/usr/bin/env bash
BLOCK_ARGS="10 20"
FILE_NAME="file.txt"
COPY_DESTINATION="file-dest.txt"

GENERATE_ARGS="$FILE_NAME $BLOCK_ARGS"
SORT_ARGS="$FILE_NAME $BLOCK_ARGS lib"
COPY_ARGS="$FILE_NAME $COPY_DESTINATION $BLOCK_ARGS lib"

./cmake-build-debug/cw02 generate $GENERATE_ARGS
./cmake-build-debug/cw02 sort $SORT_ARGS
./cmake-build-debug/cw02 copy $COPY_ARGS
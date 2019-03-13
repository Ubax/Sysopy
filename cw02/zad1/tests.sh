#!/usr/bin/env bash

RESULT_FILE_NAME="wyniki.txt"

echo "JAKUB TAKCZ - ZADANIE 1" > $RESULT_FILE_NAME

for size in 1 4 512 1024 4096  8192
do
    for try in 100 1000
    do
        FILE_NAME="file-$size-$try.txt"
        COPY_DESTINATION="file-dest-$size-$try.txt"
        BLOCK_ARGS="$try $size"

        GENERATE_ARGS="$FILE_NAME $BLOCK_ARGS"
        SORT_LIB_ARGS="$FILE_NAME $BLOCK_ARGS"
        SORT_SYS_ARGS="$COPY_DESTINATION $BLOCK_ARGS"
        COPY_ARGS="$FILE_NAME $COPY_DESTINATION $BLOCK_ARGS"

        echo -e "BLOCK SIZE: $size \t NUMBER OF BLOCKS: $try ----" >> $RESULT_FILE_NAME

        ./cmake-build-debug/cw02 generate $GENERATE_ARGS >> $RESULT_FILE_NAME

        ./cmake-build-debug/cw02 copy $COPY_ARGS lib >> $RESULT_FILE_NAME
        ./cmake-build-debug/cw02 copy $COPY_ARGS sys >> $RESULT_FILE_NAME

        ./cmake-build-debug/cw02 sort $SORT_LIB_ARGS lib >> $RESULT_FILE_NAME
        ./cmake-build-debug/cw02 sort $SORT_SYS_ARGS sys >> $RESULT_FILE_NAME
    done
done
#!/usr/bin/env bash

RESULT_FILE_NAME="wyniki-copy.txt"

echo "JAKUB TAKCZ - ZADANIE 1" > $RESULT_FILE_NAME
echo -e "Function\tUser Time\tSystem Time" >> $RESULT_FILE_NAME

for size in 1 4 512 1024 4096  8192
do
    for try in 20000 30000
    do
        FILE_NAME="file-copy-$size-$try.txt"
        COPY_DESTINATION="file-copy-dest-$size-$try.txt"
        BLOCK_ARGS="$try $size"

        GENERATE_ARGS="$FILE_NAME $BLOCK_ARGS"
        COPY_ARGS="$FILE_NAME $COPY_DESTINATION $BLOCK_ARGS"

        echo -e "BLOCK SIZE: $size \t NUMBER OF BLOCKS: $try ----" >> $RESULT_FILE_NAME

        ./cmake-build-debug/cw02 generate $GENERATE_ARGS > /dev/null

        ./cmake-build-debug/cw02 copy $COPY_ARGS lib >> $RESULT_FILE_NAME
        ./cmake-build-debug/cw02 copy $COPY_ARGS sys >> $RESULT_FILE_NAME
    done
done

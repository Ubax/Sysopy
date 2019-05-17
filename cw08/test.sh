#!/bin/bash

filters=(3 10 65)
threads=(1 2 4 8)
modes=(block interleaved)

date > Times.txt

for filter in ${filters[*]}
do
  for thread in ${threads[*]}
  do
    for mode in ${modes[*]}
    do
      ./main $thread $mode img/airplane.pgm filters/filter$filter img/out-airplane-$filter.pgm >> Times.txt
      echo "$filter $thread $mode"
    done
  done
done

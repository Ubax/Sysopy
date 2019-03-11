#!/usr/bin/env bash
args="1000 "
max=10
for i in `seq 1 5`
do
	args="$args search_directory ~ \"*.txt\" tmp.txt "
	for j in `seq 1 10`
	do
	    for k in `seq 0 $max`
	    do
	        args="$args add_to_table tmp.txt "
	    done
	    for k in `seq 0 $max`
	    do
	        args="$args remove_block $k"
	    done
	done
done
for i in `seq 1 4`
do
	./cw01_static $args
done
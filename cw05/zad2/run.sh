#!/usr/bin/env bash
for i in `seq 1 10`; do
    ./slave test 7 &
done

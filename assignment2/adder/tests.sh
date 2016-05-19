#!/bin/bash
#This script runs the adder app with specific configs, timing them all

for p in 8 9 10
do
    for K in 1 2 3 4 5 6 7 8 9 10 15 20 30
    do
        i=0
        N=$((10**p))
        echo "Testing for N=$N and K=$K..."
        FILE=logs/exec_times-$p-$K.log
        if [ -f $FILE ]
        then
            rm $FILE
        fi
        touch $FILE
        while [ $i -lt 10 ]
        do
            (time ./app_adder $N $K) &>> $FILE
            ((i+=1))
        done
    done
done

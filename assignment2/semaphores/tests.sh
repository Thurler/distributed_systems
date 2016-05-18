#!/bin/bash
#This script runs the semaphores app with specific configs, timing them all

for N in 2 4 8 16 32
do
    for P in 1 2 5 10
    do
        for C in 1 2 5 10
        do
            i=0
            echo "Testing for N=$N, P=$P and C=$C..."
            FILE=logs/exec_times-$N-$P-$C.log
            if [ -f $FILE ]
            then
                rm $FILE
            fi
            touch $FILE
            while [ $i -lt 10 ]
            do
                (time ./app_semaphores $N $P $C) &>> $FILE
                ((i+=1))
            done
        done
    done
done

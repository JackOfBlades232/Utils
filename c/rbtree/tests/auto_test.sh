#!/bin/sh
# 4_22/auto_test.sh
# script for running randomly generated tests for insertion 
# ( param 1 = num of tests )

# compile the test generating program
gcc -Wall -g generate_test.c -o generate_test.out

# compile the test engine, and link it to rbtree (must be pre-compiled)
gcc -Wall -g -c test_engine.c
gcc -Wall -g test_engine.o ../rbtree.o -o test_engine.out

# for the specified amount of time generate test and redirect it's output
# to the test engine
I=1
while [ $I -le $1 ]; do
    ./generate_test.out $I | ./test_engine.out
    I=$(( I + 1 ))
done

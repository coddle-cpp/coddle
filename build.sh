#!/bin/bash -x
for src in *.cpp
do
    clang++ -Wall -pipe -march=native -gdwarf-3 -pthread -g -std=c++17 -c $src -o $src.o &
done
wait


for obj in *.o
do
    objs="$objs $obj"
done

clang++ $objs -o coddle -pthread

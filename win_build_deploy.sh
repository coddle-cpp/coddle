#!/bin/bash -x
for src in *.cpp
do
    clang++ -Wall -pipe -march=native -gdwarf-3 -pthread -g -std=c++17 -c $src -o $src.o  -Xclang -flto-visibility-public-std &
done
wait


for obj in *.o
do
    objs="$objs $obj"
done

clang++ $objs -o coddle -pthread -Xclang -flto-visibility-public-std

install coddle /usr/bin/

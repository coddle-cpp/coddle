#!/bin/bash
for src in *.cpp
do
    echo clang++ -Wall -pipe -march=native -gdwarf-3 -pthread -g -std=c++17 -c $src -o $src.o &
    clang++ -Wall -pipe -march=native -gdwarf-3 -pthread -g -std=c++17 -c $src -o $src.o &
done
wait


for obj in *.o
do
    objs="$objs $obj"
done

echo clang++ $objs -o coddle -pthread
clang++ $objs -o coddle -pthread

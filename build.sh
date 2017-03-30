#!/bin/bash
for src in *.cpp
do
    echo g++ -Wall -pipe -march=native -gdwarf-3 -pthread -g -std=c++1y -c $src -o $src.o &
    g++ -Wall -pipe -march=native -gdwarf-3 -pthread -g -std=c++1y -c $src -o $src.o &
done
wait


for obj in *.o
do
    objs="$objs $obj"
done

echo g++ $objs -ldl -o coddle -pthread
g++ $objs -ldl -o coddle -pthread

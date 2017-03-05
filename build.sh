#!/bin/bash
set -x
for src in *.cpp
do
    g++ -Wall -pipe -march=native -gdwarf-3 -pthread -g -std=c++1y -c $src -o $src.o &
done
wait


for obj in *.o
do
    if [ $obj != "main.cpp.o" ]
    then
        objs="$objs $obj"
    fi
done

g++ -pthread $objs main.cpp.o -o coddle
ar rv libcoddle.a $objs
mkdir -p ~/.coddle/bin
mkdir -p ~/.coddle/lib
mkdir -p ~/.coddle/include/coddle/
current=$(realpath .)
ln -sf $current/coddle ~/.coddle/bin/
ln -sf $current/libcoddle.a ~/.coddle/lib/
for h in *.hpp
do
    ln -sf $current/$h ~/.coddle/include/coddle/
done
if ! grep coddle ~/.bashrc
then
    echo Adding alias
    echo alias coddle=~/.coddle/bin/coddle >> ~/.bashrc
fi

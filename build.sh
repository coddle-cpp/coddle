#!/bin/bash
for src in *.cpp
do
    echo g++ -Wall -pipe -march=native -gdwarf-3 -pthread -g -std=c++1y -c $src -o $src.o &
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

echo g++ $objs main.cpp.o -o coddle -pthread
g++ $objs main.cpp.o -o coddle -pthread
echo ar rv libcoddle.a $objs
ar rv libcoddle.a $objs
mkdir -p ~/.coddle/bin
mkdir -p ~/.coddle/lib
mkdir -p ~/.coddle/include/coddle/
current=$(pwd)
install $current/coddle ~/.coddle/bin/
install $current/libcoddle.a ~/.coddle/lib/
for h in *.hpp
do
    install $current/$h ~/.coddle/include/coddle/
done

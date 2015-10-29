#!/bin/bash
set -x
g++ -Wall -pipe -march=native -gdwarf-3 -pthread -g -std=c++1y *.cpp -o coddle

#!/bin/bash
set -x
install `dirname $0`/coddle /usr/local/bin/coddle
mkdir -p /usr/local/include/coddle
current=$(pwd)
for h in *.hpp
do
    install $current/$h /usr/local/include/coddle/
done


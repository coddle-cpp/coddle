#!/bin/bash
set -x
install `dirname $0`/coddle /usr/local/bin/coddle
mkdir -p /usr/local/include/coddle
current=$(pwd)
install $current/config.hpp /usr/local/include/coddle/


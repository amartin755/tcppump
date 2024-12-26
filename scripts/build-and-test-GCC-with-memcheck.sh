#!/bin/bash

set -e

BUILDDIRNAME=build
SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
PROJROOT=$(realpath $SCRIPTPATH/..)


# clean configure and build
###############################################################################
rm -rf $PROJROOT/$BUILDDIRNAME
cd $PROJROOT
cmake -B $BUILDDIRNAME -DWITH_UNITTESTS=ON -DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo
cmake --build build -j


# execute all ctest test cases that don't need raw privileges
###############################################################################
cd $PROJROOT/$BUILDDIRNAME
ctest -T memcheck -E "online-*"

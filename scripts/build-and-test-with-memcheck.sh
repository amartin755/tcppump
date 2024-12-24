#!/bin/bash

set -e

BUILDDIRNAME=build
SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
PROJROOT=$(realpath $SCRIPTPATH/..)


# clean configure and build
###############################################################################
rm -rf $PROJROOT/$BUILDDIRNAME
cd $PROJROOT
cmake -B $BUILDDIRNAME -DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo
cmake --build build -j


# run unit tests (working dir should be "bin" because of created tmp files)
###############################################################################
cd $PROJROOT/bin && valgrind --leak-check=yes ./unittest


# execute all ctest test cases that need raw privileges
###############################################################################
cd $PROJROOT/$BUILDDIRNAME
ctest -T memcheck -E "online-*"

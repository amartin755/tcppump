#!/bin/bash

set -e

BUILDDIRNAME="${1:-build}"
SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
PROJROOT=$(realpath $SCRIPTPATH/..)


# clean configure and build
###############################################################################
$SCRIPTPATH/utils/clean-build.sh -t -C RelWithDebInfo -T CLANG -B $BUILDDIRNAME


# execute all ctest test cases that don't need raw privileges (exclude diff)
###############################################################################
cd $PROJROOT/$BUILDDIRNAME
ctest -T memcheck -E "--diff|online-*" -j

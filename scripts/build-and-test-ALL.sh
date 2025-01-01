#!/bin/bash

set -e

BUILDDIRNAME=build
SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
PROJROOT=$(realpath $SCRIPTPATH/..)

cd $PROJROOT

$SCRIPTPATH/build-and-test-GCC.sh
$SCRIPTPATH/build-and-test-GCC-with-ASAN.sh
$SCRIPTPATH/build-and-test-GCC-with-UBSAN.sh
$SCRIPTPATH/build-and-test-GCC-with-memcheck.sh

$SCRIPTPATH/build-and-test-CLANG.sh
$SCRIPTPATH/build-and-test-CLANG-with-ASAN.sh
$SCRIPTPATH/build-and-test-CLANG-with-UBSAN.sh
$SCRIPTPATH/build-and-test-CLANG-with-memcheck.sh

echo "----- SUCCESS ----"

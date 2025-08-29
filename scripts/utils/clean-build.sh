#!/bin/bash

set -e


SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
PROJROOT=$(realpath $SCRIPTPATH/../..)

BUILDDIRNAME=build
BUILD_CONIFG=Debug
UNITTESTS=OFF
ASAN=OFF
UBSAN=OFF
WERROR=OFF
CPPCHECK=OFF
TOOLCHAIN=GCC

usage() {
    echo "Usage: $0 [options]"
    echo
    echo "Options:"
    echo "  -t             Compile with unit tests"
    echo "  -a             Enable ASAN"
    echo "  -u             Enable UBSAN"
    echo "  -c             Enable cppcheck"
    echo "  -e             Compile with -Werror"
    echo "  -C <config>    Build config (Debug, Release, ...)"
    echo "  -T <toolchain> Toolchain (GCC, CLANG)"
    echo "  -B <dir>       cmake build directory"
    echo "  -h             Display help information"
    echo
    }

while getopts htauceC:T:B: arg
do
   case $arg in
       t) UNITTESTS=ON;;
       a) ASAN=ON;;
       u) UBSAN=ON;;
       c) CPPCHECK=ON;;
       e) WERROR=ON;;
       C) BUILD_CONIFG=$OPTARG;;
       T) TOOLCHAIN=$OPTARG;;
       B) BUILDDIRNAME=$OPTARG;;
       h) usage
          exit 0;;
   esac
done

#echo -DWITH_UNITTESTS=$UNITTESTS -DWITH_ASAN=$ASAN -DWITH_UBSAN=$UBSAN -DWITH_WERROR=$WERROR -DCMAKE_BUILD_TYPE:STRING=$BUILD_CONIFG TOOLCHAIN=$TOOLCHAIN BUILDDIRNAME=$BUILDDIRNAME

if [ "$TOOLCHAIN" = "CLANG" ]
then
    export CC=/usr/bin/clang
    export CXX=/usr/bin/clang++
fi

cd $PROJROOT
rm -rf $PROJROOT/$BUILDDIRNAME
cmake -B $BUILDDIRNAME  -DWITH_UNITTESTS=$UNITTESTS -DWITH_ASAN=$ASAN -DWITH_UBSAN=$UBSAN -DWITH_CPPCHECK=$CPPCHECK -DWITH_WERROR=$WERROR -DCMAKE_BUILD_TYPE:STRING=$BUILD_CONIFG
cmake --build $BUILDDIRNAME -j
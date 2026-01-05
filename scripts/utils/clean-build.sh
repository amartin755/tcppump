#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-only
###############################################################################
#
# TCPPUMP <https://github.com/amartin755/tcppump>
# Copyright (C) 2012-2026 Andreas Martin (netnag@mailbox.org)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################

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
#!/bin/bash

set -e

BUILDDIRNAME=build
SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
PROJROOT=$(realpath $SCRIPTPATH/..)


# before we start, enforce successful sudo
###############################################################################
sudo -k
echo "NOTE: need sudo for setting of raw-capabilities for tcppump binary"
sudo true


# clean configure and build
###############################################################################
rm -rf $PROJROOT/$BUILDDIRNAME
cd $PROJROOT
cmake -B $BUILDDIRNAME  -DWITH_UNITTESTS=ON -DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo
cmake --build build -j16


# run all ctest cases
###############################################################################

# set raw capabilities
sudo setcap cap_net_raw+eip $PROJROOT/bin/tcppump

# execute ctest
cd $PROJROOT/$BUILDDIRNAME
ctest

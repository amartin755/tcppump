#!/bin/bash

set -e

BUILDDIRNAME=build
SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
PROJROOT=$(realpath $SCRIPTPATH/..)
DUMPABLE=$(sysctl -n fs.suid_dumpable)


# before we start, enforce successful sudo
###############################################################################
sudo -k
echo "NOTE: need sudo for setting of raw-capabilities for tcppump binary and ASAN suid_dumpable tweaks"
sudo true


# clean configure and build
###############################################################################
rm -rf $PROJROOT/$BUILDDIRNAME
cd $PROJROOT
cmake -B $BUILDDIRNAME -DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo -DWITH_ASAN=ON
cmake --build build -j16


# run unit tests (working dir should be "bin" because of created tmp files)
###############################################################################
cd $PROJROOT/bin && ./unittest


# run all ctest cases
###############################################################################

# set raw capabilities
sudo setcap cap_net_raw+eip $PROJROOT/bin/tcppump

# disable coredumps for priviledged processes (otherwise ASAN will refuse to run)
if [ $DUMPABLE -gt 1 ]
then
    sudo sysctl -w fs.suid_dumpable\=0
fi

# disable exit on error again to ensure suid_dumpable is always restored
set +e

# execute ctest
cd $PROJROOT/$BUILDDIRNAME
ctest

# restore suid coredump settings
if [ $DUMPABLE -gt 1 ]
then
    sudo sysctl -q -w fs.suid_dumpable\=$DUMPABLE
fi
#!/bin/bash

set -e

BUILDDIRNAME=build
SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
PROJROOT=$(realpath $SCRIPTPATH/..)
DUMPABLE=$(sysctl -n fs.suid_dumpable)



# clean configure and build
###############################################################################
$SCRIPTPATH/utils/clean-build.sh -t -a -C RelWithDebInfo -T CLANG -B $BUILDDIRNAME


# run all ctest cases
###############################################################################

# set raw capabilities
echo "NOTE: need sudo for setting of raw-capabilities for tcppump binary"
sudo setcap cap_net_raw+eip $PROJROOT/bin/tcppump

# disable coredumps for priviledged processes (otherwise ASAN will refuse to run)
if [ $DUMPABLE -gt 1 ]
then
    echo "NOTE: need sudo because ASAN expects /proc/sys/fs/suid_dumpable != 2"
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
    echo "NOTE: need sudo for restoring of /proc/sys/fs/suid_dumpable"
    sudo sysctl -q -w fs.suid_dumpable\=$DUMPABLE
fi

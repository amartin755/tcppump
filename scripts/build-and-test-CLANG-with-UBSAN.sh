#!/bin/bash

set -e

BUILDDIRNAME="${1:-build}"
SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
PROJROOT=$(realpath $SCRIPTPATH/..)



# clean configure and build
###############################################################################
$SCRIPTPATH/utils/clean-build.sh -t -u -C RelWithDebInfo -T CLANG -B $BUILDDIRNAME


# run all ctest cases
###############################################################################

# set raw capabilities
echo "NOTE: need sudo for setting of raw-capabilities for tcppump binary"
sudo setcap cap_net_raw+eip $PROJROOT/bin/tcppump

# execute ctest
cd $PROJROOT/$BUILDDIRNAME
ctest

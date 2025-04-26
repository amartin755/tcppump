#!/bin/bash

set -e

BUILDDIRNAME=build
SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
PROJROOT=$(realpath $SCRIPTPATH/..)

cd $PROJROOT


echo "----- GCC Debug ----"
$SCRIPTPATH/clean-build.sh -t -c -C Debug -T GCC -B $BUILDDIRNAME

echo "----- GCC Release ----"
$SCRIPTPATH/clean-build.sh -t -c -C Release -T GCC -B $BUILDDIRNAME


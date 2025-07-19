#!/bin/bash

set -e

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
PROJROOT=$(realpath $SCRIPTPATH/..)
CURRDIR=$PWD


cleanup()
{
    if [ -n "${WORKDIR:-}" ] && [ -d "$WORKDIR" ]; then
        rm -rf $WORKDIR
    fi
}
trap cleanup EXIT


if [ $# -ne 1 ]; then
    echo "Usage: $0 <source-tarball>"
    exit 1
fi

TARBALL="$1"
if [ ! -f "$TARBALL" ]; then
    echo "'$TARBALL' does not exist."
    exit 1
fi

VERSION=$(echo $(basename "$TARBALL") | sed -E 's/^tcppump_([0-9]+\.[0-9]+\.[0-9]+)\.tar\.gz$/\1/')

ORIG_TARBALL=tcppump_$VERSION.orig.tar.gz
WORKDIR=$(mktemp -d)
cp $TARBALL $WORKDIR/$ORIG_TARBALL
cd $WORKDIR
tar xf $ORIG_TARBALL
if [ -d tcppump-$VERSION ]; then
    cd tcppump-$VERSION
    if grep -xq "VERSION $VERSION" ./VERSION; then
        debuild -us -uc
        cp ../tcppump_$VERSION*.deb $CURRDIR
    else
        echo "$VERSION doesn't mach content of $TARBALL"
        exit 1
    fi
else
    echo "'tcppump-$VERSION' does not exist"
    exit 1
fi

cd $CURRDIR

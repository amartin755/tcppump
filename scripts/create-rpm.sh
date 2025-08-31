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


if [ $# -ne 2 ]; then
    echo "Usage: $0 <source-tarball> <destination-dir>"
    exit 1
fi

TARBALL="$1"
if [ ! -f "$TARBALL" ]; then
    echo "'$TARBALL' does not exist."
    exit 1
fi
DEST=$(realpath $2)
if [ ! -d "$DEST" ]; then
    echo "'$DEST' does not exist."
    exit 1
fi

VERSION=$(echo $(basename "$TARBALL") | sed -E 's/^tcppump_([0-9]+\.[0-9]+\.[0-9]+)\.tar\.gz$/\1/')

WORKDIR=$(mktemp -d)
mkdir -p $WORKDIR/SOURCES
mkdir -p $WORKDIR/SPECS
cp $TARBALL $WORKDIR/SOURCES
cd $WORKDIR/SPECS
tar --strip-components=2 -xvzf $WORKDIR/SOURCES/$(basename "$TARBALL") tcppump-$VERSION/rpm/tcppump.spec
rpmbuild -ba tcppump.spec --define "_topdir $WORKDIR"
find ../RPMS -type f -name "*.rpm" -exec cp {} "$DEST" \;
find ../SRPMS -type f -name "*.rpm" -exec cp {} "$DEST" \;

cd $CURRDIR

#!/bin/bash

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
PROJROOT=$(realpath $SCRIPTPATH/..)
UTILSPATH=$(realpath $SCRIPTPATH/utils)

DEST=$PROJROOT/RELEASE

VERSION=$($UTILSPATH/get-project-version.py < $PROJROOT/CMakeLists.txt)

rm -rf $DEST
mkdir $DEST

if ! $SCRIPTPATH/create-source-tarball.sh $DEST; then
    echo "kdkdkdk"
    exit 1
fi
TARBALL=$(basename $DEST)/tcppump_$VERSION.tar.gz
DEST=$(basename $DEST)
echo $TARBALL $DEST

# TODO some kind of table makes more sense
mkdir $DEST/debian.trixie
$SCRIPTPATH/run-container.sh debian.trixie scripts/create-deb.sh $TARBALL $DEST/debian.trixie
mkdir $DEST/debian.bookworm
$SCRIPTPATH/run-container.sh debian.bookworm scripts/create-deb.sh $TARBALL $DEST/debian.bookworm
mkdir $DEST/ubuntu.lts
$SCRIPTPATH/run-container.sh ubuntu.lts scripts/create-deb.sh $TARBALL $DEST/ubuntu.lts
mkdir $DEST/ubuntu.latest
$SCRIPTPATH/run-container.sh ubuntu.latest scripts/create-deb.sh $TARBALL $DEST/ubuntu.latest
mkdir $DEST/fedora.42
$SCRIPTPATH/run-container.sh fedora.42 scripts/create-rpm.sh $TARBALL $DEST/fedora.42
mkdir $DEST/suse-leap.15.6
$SCRIPTPATH/run-container.sh suse-leap.15.6 scripts/create-rpm.sh $TARBALL $DEST/suse-leap.15.6

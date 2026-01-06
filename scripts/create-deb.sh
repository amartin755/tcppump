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

ORIG_TARBALL=tcppump_$VERSION.orig.tar.gz
WORKDIR=$(mktemp -d)
cp $TARBALL $WORKDIR/$ORIG_TARBALL
cd $WORKDIR
tar xf $ORIG_TARBALL
if [ -d tcppump-$VERSION ]; then
    cd tcppump-$VERSION
    if grep -xq "VERSION $VERSION" ./VERSION; then
        # build debian binary package
        debuild --no-sign --build=binary
        
        # determine filename pattern of the resulting deb package
        FILE_NAME="$(dpkg-parsechangelog -S Source)_$(dpkg-parsechangelog -S Version)_$(dpkg-architecture -qDEB_HOST_ARCH)"

        # copy debian packages and build info to destination
        find .. -maxdepth 1 -type f -name "$FILE_NAME.deb" -exec cp {} $DEST \;
        find .. -maxdepth 1 -type f -name "$FILE_NAME.build" -exec cp {} $DEST \;
        find .. -maxdepth 1 -type f -name "$FILE_NAME.buildinfo" -exec cp {} $DEST \;
        cp /etc/os-release "$DEST/$FILE_NAME.os-release"
    else
        echo "$VERSION doesn't mach content of $TARBALL"
        exit 1
    fi
else
    echo "'tcppump-$VERSION' does not exist"
    exit 1
fi

cd $CURRDIR

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

OUR_VERSION=$(echo $(basename "$TARBALL") | sed -E 's/^tcppump_([0-9]+\.[0-9]+\.[0-9]+)\.tar\.gz$/\1/')

CURRENT_DIST="$(rpm --eval '%{?dist}')"
RPMBUILD_DEFINES=()
if [[ -z "$CURRENT_DIST" ]]; then
    if [[ -r /etc/os-release ]]; then
        source /etc/os-release
    else
        echo "ERROR: /etc/os-release not found"
        exit 1
    fi

    case "$ID" in
        fedora)
            DIST=".fc${VERSION_ID}"
            ;;
        rhel|centos|rocky|almalinux)
            DIST=".el${VERSION_ID%%.*}"
            ;;
        *leap)
            DIST=".leap${VERSION_ID}"
            ;;
        *tumbleweed)
            DIST=".tumbleweed"
            ;;
        *)
            DIST=""
            ;;
    esac

    if [[ -n "$DIST" ]]; then
        RPMBUILD_DEFINES+=(--define "dist ${DIST}")
    fi
fi

# prepare RPM build environment
WORKDIR=$(mktemp -d)
mkdir -p $WORKDIR/SOURCES
mkdir -p $WORKDIR/SPECS
cp $TARBALL $WORKDIR/SOURCES
cd $WORKDIR/SPECS
tar --strip-components=2 -xvzf $WORKDIR/SOURCES/$(basename "$TARBALL") tcppump-$OUR_VERSION/rpm/tcppump.spec

# determine filename pattern of the resulting rpm package
FILE_NAME=$(rpmspec "${RPMBUILD_DEFINES[@]}" --query --qf '%{NAME}-%{VERSION}-%{RELEASE}.%{ARCH}' tcppump.spec)

# build RPM package and log output
rpmbuild "${RPMBUILD_DEFINES[@]}" -bb tcppump.spec --define "_topdir $WORKDIR" | tee "$DEST/$FILE_NAME.build"

# copy RPM package and build infos to destination
find ../RPMS -type f -name "$FILE_NAME.rpm" -exec cp {} $DEST \;
rpm -qa > "$DEST/$FILE_NAME.buildinfo"
cp /etc/os-release "$DEST/$FILE_NAME.os-release"

cd $CURRDIR

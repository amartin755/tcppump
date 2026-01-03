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

WORKDIR=$(mktemp -d)
mkdir -p $WORKDIR/SOURCES
mkdir -p $WORKDIR/SPECS
cp $TARBALL $WORKDIR/SOURCES
cd $WORKDIR/SPECS
tar --strip-components=2 -xvzf $WORKDIR/SOURCES/$(basename "$TARBALL") tcppump-$OUR_VERSION/rpm/tcppump.spec
rpmbuild "${RPMBUILD_DEFINES[@]}" -ba tcppump.spec --define "_topdir $WORKDIR" | tee "$DEST/tcppump-$OUR_VERSION.build"
find ../RPMS -type f -name "*.rpm" -exec cp {} "$DEST" \;
find ../SRPMS -type f -name "*.rpm" -exec cp {} "$DEST" \;
rpm -qa > "$DEST/tcppump-$OUR_VERSION.buildinfo"

cd $CURRDIR

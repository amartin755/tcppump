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
set -euo pipefail
shopt -s nullglob

# Determine important paths
SCRIPTPATH="$(cd -- "$(dirname "$0")" >/dev/null 2>&1 && pwd -P)"
PROJROOT="$(realpath "$SCRIPTPATH/..")"
UTILSPATH="$(realpath "$SCRIPTPATH/utils")"
DEST="$PROJROOT/RELEASE"

OPT_SILENT=1

usage()
{
    echo "Usage: $0 [options] list-of-docker-images"
    echo
    echo "Options:"
    echo "  -v             print build output"
    echo "  -h             Display help information"
    echo
}

while getopts "hva" option; do
    case $option in
        v) OPT_SILENT=0;;
        h) usage
           exit 0;;
    esac
done
shift $((OPTIND - 1))

# Get version
VERSION="$("$UTILSPATH/get-project-version.py" < "$PROJROOT/CMakeLists.txt")"
TARBALL_NAME="tcppump_${VERSION}.tar.gz"

# Clean and prepare release directory
rm -rf "$DEST"
mkdir -p "$DEST"

# Create source tarball
if ! "$SCRIPTPATH/create-source-tarball.sh" "$DEST"; then
    echo "error: Failed to create source tarball" >&2
    exit 1
fi

# Compute relative paths for tarball
TARBALL_PATH="$DEST/$TARBALL_NAME"
TARBALL_RELATIVE=$(realpath --relative-to="$PROJROOT" "$TARBALL_PATH")

echo "Source tarball created: $TARBALL_RELATIVE"


# Get a list of all tcppump images
if [ "$#" -eq 0 ]; then
    DISTROS=($(docker image ls tcppump --format "{{.Tag}}"))
else
    DISTROS=("$@")
fi

package()
{
    if (( OPT_SILENT == 1 )); then
        "$@" >/dev/null
    else
        "$@"
    fi
}

pkg_type() {
    local distro="$1"

    case "$distro" in
    debian*|ubuntu*)
        echo "deb"
        ;;
    suse*|opensuse*)
        echo "rpm"
        ;;
    arch*)
        echo "pkg.tar.zst"
        ;;
    fedora*|centos*|rhel*)
        echo "rpm"
        ;;
    *)
        echo "unknown"
        return 1
        ;;
    esac
}

# Loop through distros
echo "# build packages"
FINISHED_BUILDS=()
for distro in "${DISTROS[@]}"; do
    if docker image inspect "tcppump:$distro" >/dev/null 2>&1; then

        pkg_type=$(pkg_type $distro)
        out_dir="$DEST/$distro"
        out_dir_relative=$(realpath --relative-to="$PROJROOT" "$out_dir")

        printf "%-30s" "$distro ($pkg_type)"
        # check if we have a script for this package-type
        if [ ! -f $SCRIPTPATH/create-${pkg_type}.sh ]; then
            echo " skipped"
            continue
        fi
        mkdir -p "$out_dir"

        # build the package
        package "$SCRIPTPATH/run-container.sh" "$distro" "scripts/create-${pkg_type}.sh" "$TARBALL_RELATIVE" "$out_dir_relative"
        if (( $? != 0 )); then
            echo "error: Failed to create package for $distro" >&2
            exit 1
        fi
        FINISHED_BUILDS+=("$out_dir")
        echo " done"
    else
        echo "Unknown docker image $distro"
        exit 1
    fi
done

# Sign and create SHA256 sums for all built packages
echo "# Sign packages and create SHA256 sums"
for build in "${FINISHED_BUILDS[@]}"; do
    cd "$build"
    for f in *.deb *.rpm; do
        gpg --verbose --detach-sig --yes -a "$f"
        sha256sum "$f" | tee "$f.sha256"
    done
done

echo
echo "All packages built successfully in: $(realpath --relative-to="$PROJROOT" "$DEST")"

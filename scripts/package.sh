#!/bin/bash
set -euo pipefail

# Determine important paths
SCRIPTPATH="$(cd -- "$(dirname "$0")" >/dev/null 2>&1 && pwd -P)"
PROJROOT="$(realpath "$SCRIPTPATH/..")"
UTILSPATH="$(realpath "$SCRIPTPATH/utils")"
DEST="$PROJROOT/RELEASE"

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

# Define distros and packaging type
declare -A DISTROS=(
    [debian.stable]=deb
    [debian.oldstable]=deb
    [debian.oldoldstable]=deb
    [ubuntu.rolling]=deb
    [ubuntu.latest]=deb
    [fedora.latest]=rpm
    [suse-leap.latest]=rpm
    [suse-tumbleweed.latest]=rpm
)

# Loop through distros
for distro in "${!DISTROS[@]}"; do
    pkg_type="${DISTROS[$distro]}"
    out_dir="$DEST/$distro"
    out_dir_relative=$(realpath --relative-to="$PROJROOT" "$out_dir")

    echo "==============================================================="
    echo " Building package for $distro ($pkg_type)"
    echo "==============================================================="

    mkdir -p "$out_dir"
    if ! "$SCRIPTPATH/run-container.sh" "$distro" "scripts/create-${pkg_type}.sh" "$TARBALL_RELATIVE" "$out_dir_relative"; then
        echo "error: Failed to create package for $distro" >&2
        exit 1
    fi

    # store distro version infos
    "$SCRIPTPATH/run-container.sh" "$distro" cp /etc/os-release "$out_dir_relative"

    # sign all created packages
    for i in $out_dir/*.${pkg_type}; do
        [ -f "$i" ] || break
        gpg --verbose --detach-sig --yes $i
    done
done

# TODO: collect all logs (*.build *.buildinfo) -> tar.gz -> sign

echo
echo "All packages built successfully in: $(realpath --relative-to="$PROJROOT" "$DEST")"

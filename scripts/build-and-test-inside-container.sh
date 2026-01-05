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

# Get script and project paths
SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
PROJROOT=$(realpath "$SCRIPTPATH/..")
BUILDDIR=build-docker

cleanup()
{
    if [ -n "${BUILDDIR:-}" ] && [ -d "$BUILDDIR" ]; then
        rm -rf $BUILDDIR
    fi
}
trap cleanup EXIT

# Get a list of all tcppump images
IMAGES=($(docker image ls --format "{{.Repository}}:{{.Tag}}" | grep '^tcppump'))

# loop over images and run build in each
for IMAGE_TAG in "${IMAGES[@]}"; do
    echo "Running container for image: $IMAGE_TAG"

    # strip 'tcppump:' prefix for tag because run-container.sh expects just the tag
    TAG=${IMAGE_TAG#tcppump:}

    # build and test with both GCC and CLANG inside the container
    $SCRIPTPATH/run-container.sh "$TAG" scripts/build-and-test-GCC.sh   $BUILDDIR
    $SCRIPTPATH/run-container.sh "$TAG" scripts/build-and-test-CLANG.sh $BUILDDIR
    echo "----------------------------------------"
done

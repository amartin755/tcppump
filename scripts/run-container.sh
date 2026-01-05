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

# Get a list of all tcppump images
IMAGES=($(docker image ls --format "{{.Repository}}:{{.Tag}}" | grep '^tcppump'))

select_image() {
    if [ ${#IMAGES[@]} -eq 0 ]; then
        echo "No 'tcppump' images found! Please build or pull one first."
        exit 1
    fi

    if [ ${#IMAGES[@]} -eq 1 ]; then
        IMAGE_TAG="${IMAGES[0]}"
        echo "Only one image found: $IMAGE_TAG"
        return
    fi

    echo "Available tcppump images:"
    for i in "${!IMAGES[@]}"; do
        echo "[$i] ${IMAGES[$i]}"
    done

    while true; do
        read -p "Please select an image (number): " idx
        if [[ "$idx" =~ ^[0-9]+$ ]] && [ "$idx" -ge 0 ] && [ "$idx" -lt "${#IMAGES[@]}" ]; then
            IMAGE_TAG="${IMAGES[$idx]}"
            break
        else
            echo "Invalid input. Enter a number between 0 and $((${#IMAGES[@]}-1))."
        fi
    done
}

# Check if first argument is a valid tag
if [ $# -gt 0 ]; then
    CANDIDATE="tcppump:$1"
    if docker image inspect "$CANDIDATE" >/dev/null 2>&1; then
        IMAGE_TAG="$CANDIDATE"
        shift
    else
        select_image
    fi
else
    select_image
fi

# Volume mount (SELinux compatible)
VOLUME="-v $PROJROOT:/workdir:z"

# Run the container
docker run -it --rm $VOLUME "$IMAGE_TAG" "$@"


# non-interactive variant

# if [ $# -lt 1 ]; then
#     echo "Usage: $0 <image-tag> [docker run options...]"
#     echo "available tcppump images:"
#     docker image ls tcppump --format "{{.Tag}}"
#     exit 1
# fi

# SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
# PROJROOT=$(realpath $SCRIPTPATH/..)

# # first parameter = Image-Tag
# IMAGE_TAG=tcppump:$1
# shift

# # mount project-root directory, SELinux-compatible
# VOLUME="-v $PROJROOT:/workdir:z"

# # start container, add remaining parameters
# docker run -it --rm $VOLUME "$IMAGE_TAG" "$@"

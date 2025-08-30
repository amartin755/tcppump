#!/bin/bash
set -e

# set UID/GID
UID_ARG=$(id -u)
GID_ARG=$(id -g)

# iterate over all Dockerfile.* in current directory
for dockerfile in Dockerfile.*; do

    [ -e "$dockerfile" ] || continue

    tag="tcppump:${dockerfile#Dockerfile.}"

    echo "==============================================================="
    echo " Building $dockerfile with tag $tag"
    echo "==============================================================="

    docker build --build-arg UID=$UID_ARG --build-arg GID=$GID_ARG -f "$dockerfile" -t "$tag" .
#    docker build  -f "$dockerfile" -t "$tag" .
done

echo "all images successfully built"

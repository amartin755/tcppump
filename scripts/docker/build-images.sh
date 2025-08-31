#!/bin/bash
set -e

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

# set UID/GID
UID_ARG=$(id -u)
GID_ARG=$(id -g)

# Function: get image creation time (epoch) or 0 if not exists
get_image_time() {
    local image_tag=$1
    local created
    created=$(docker image inspect --format='{{.Created}}' "$image_tag" 2>/dev/null || echo "")
    if [ -z "$created" ]; then
        echo 0
    else
        # Convert to epoch seconds
        date --date="$created" +%s
    fi
}

# Build all Dockerfiles in docker directory
for dockerfile in $SCRIPTPATH/Dockerfile.*; do

    [ -e "$dockerfile" ] || continue
    
    tag="tcppump:${dockerfile##*/Dockerfile.}"

    # Get timestamps
    file_time=$(stat -c %Y "$dockerfile")
    image_time=$(get_image_time "$tag")
    
    echo "==============================================================="
    echo " Dockerfile: $(basename $dockerfile)"
    echo " Tag:        $tag"
    echo "==============================================================="
    if [ "$image_time" -ge "$file_time" ]; then
        echo "Skipping build: Image is up-to-date."
        continue
    fi

    docker build --build-arg UID=$UID_ARG --build-arg GID=$GID_ARG -f "$dockerfile" -t "$tag" .
#    docker build  -f "$dockerfile" -t "$tag" .
done

echo "all images successfully built"

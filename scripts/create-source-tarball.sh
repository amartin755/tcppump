#!/bin/bash

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
PROJROOT=$(realpath $SCRIPTPATH/..)

# if no destination path is provided we use the current directoy
if [ $# -gt 0 ] && [ -d "$1" ]; then
    DEST=$1
else
    DEST=$(pwd)
fi
DEST=$(realpath $DEST)

# ensure that the destination directory is outside of the source directory
if [ "$DEST" = "$PROJROOT" ] || [[ "$DEST" == "$PROJROOT"/* ]]; then
    echo "Error: Destination directory '$DEST' must not be within/below '$PROJROOT'."
    exit 1
fi

# create VERSION file
cd $PROJROOT
VERSION=$(awk 'BEGIN{RS=")"} /project[ \t]*\(/ {if (match($0, /VERSION[ \t]*([0-9]+\.[0-9]+\.[0-9]+)/, m)) print m[1]}' CMakeLists.txt)

if git rev-parse --is-inside-work-tree &> /dev/null; then
    GIT_COMMIT=$(git rev-parse HEAD)

    if [[ -n $(git status --porcelain) ]]; then
        MODIFIED="(modfied)"
    fi

    GIT_TAG=$(git describe --tags --exact-match 2>/dev/null)
fi

{
    echo "VERSION $VERSION"
    echo "COMMIT  $GIT_COMMIT $MODIFIED"
    echo "TAG     $GIT_TAG"
} > VERSION

# create TAR ball
cd $DEST
TARBALL=tcppump_$VERSION.tar.gz
tar --transform s/tcppump/tcppump-$VERSION/ \
    --exclude=bin \
    --exclude=.gitignore \
    --exclude=.gitmodules \
    --exclude=.git \
    --exclude=.vscode \
    --exclude=.github \
    --exclude=build \
    -czvf $TARBALL \
    $PROJROOT/../tcppump

sha256sum $TARBALL > $TARBALL.sha256
gpg --detach-sig --yes $TARBALL

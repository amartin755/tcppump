#!/bin/bash

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
PROJROOT=$(realpath $SCRIPTPATH/..)

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

cd $PROJROOT/..

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
    tcppump

sha256sum $TARBALL > $TARBALL.sha256
gpg --detach-sig $TARBALL

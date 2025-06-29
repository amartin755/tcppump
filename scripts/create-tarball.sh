#!/bin/bash

set -e

BUILDDIRNAME=build
SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
PROJROOT=$(realpath $SCRIPTPATH/..)

cd $PROJROOT
VERSION=$(awk 'BEGIN{RS=")"} /project[ \t]*\(/ {if (match($0, /VERSION[ \t]*([0-9]+\.[0-9]+\.[0-9]+)/, m)) print m[1]}' CMakeLists.txt)

echo "Version: $VERSION"

cd $PROJROOT/..

tar --transform s/tcppump/tcppump-$VERSION/ --exclude=bin --exclude=.gitignore --exclude=.gitmodules --exclude=.git --exclude=.vscode --exclude=.github --exclude=build  -czvf tcppump_$VERSION.orig.tar.gz tcppump

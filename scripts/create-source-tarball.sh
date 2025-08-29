#!/bin/bash

# usage: $0 [--clean] [<destination>]
#
# Options:
#   --no-clean   don't clean the working directory to the latest git state

# Arguments:
#   destination  Destination directory where the created tarball is stored.


SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
PROJROOT=$(realpath $SCRIPTPATH/..)
UTILSPATH=$(realpath $SCRIPTPATH/utils)
CLEAN_WORKDIR=1
DEST=$(pwd)

cleanup()
{
    if [ -n "${TMP_DIR:-}" ] && [ -d "$TMP_DIR" ]; then
        rm -rf $TMP_DIR
    fi
}
trap cleanup EXIT

# parse arguments
for arg in "$@"; do
    if [ "$arg" = "--no-clean" ]; then
        CLEAN_WORKDIR=0
    fi
done
if [ $# -gt 0 ]; then
    if [ "${!#}" != "--no-clean" ]; then
        DEST=${!#}
    fi
fi

DEST=$(realpath $DEST)

# copy current working directory to a temporary directory
echo "# copy current working dir to tmp dir"
TMP_DIR=$(mktemp -d)
cp -r $PROJROOT $TMP_DIR
cd $TMP_DIR/tcppump
echo "$TMP_DIR/tcppump"

# revert to last commit and remove all untracked files
if [ "$CLEAN_WORKDIR" -eq 1 ]; then
    echo "# cleanup working dir"
    git clean -d -x -f
    git reset --hard
fi

# create VERSION file based on project version and git commit and tag
echo "# collecting version and git infos"
VERSION=$($UTILSPATH/get-project-version.py < $PROJROOT/CMakeLists.txt)

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
echo -n $(cat VERSION)
echo

# add version to working dir name
cd $TMP_DIR
mv $TMP_DIR/tcppump $TMP_DIR/tcppump-$VERSION 
cd $TMP_DIR/tcppump-$VERSION

set -e
# update version in rpm spec file and add changelog
echo "# create RPM spec file"
sed -i -E "s/^(Version:[[:space:]]*).*/\1$VERSION/" "rpm/tcppump.spec"
$UTILSPATH/convert-changelog.py --package tcppump --format rpm < CHANGELOG.md >> rpm/tcppump.spec
echo "rpm/tcppump.spec"

# create debian changelog
echo "# create Debian changelog"
$UTILSPATH/convert-changelog.py --package tcppump --format debian < CHANGELOG.md > debian/changelog
echo "debian/changelog"

# create tarball, hash and signature
echo "# create tarball"
cd $TMP_DIR
TARBALL=$DEST/tcppump_$VERSION.tar.gz
tar --exclude=bin \
    --exclude=.gitignore \
    --exclude=.gitmodules \
    --exclude=.git \
    --exclude=.vscode \
    --exclude=.github \
    --exclude=build \
    -czvf $TARBALL \
    tcppump-$VERSION > /dev/null
echo $TARBALL

echo "# create sha256"
sha256sum $TARBALL | tee $TARBALL.sha256

echo "# sign tarball"
gpg --verbose --detach-sig --yes $TARBALL

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
cd $TMP_DIR/$(basename $PROJROOT)
echo "$TMP_DIR/$(basename $PROJROOT)"

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
mv $TMP_DIR/$(basename $PROJROOT) $TMP_DIR/tcppump-$VERSION 
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

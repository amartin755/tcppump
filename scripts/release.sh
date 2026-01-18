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
set -euo pipefail

SCRIPTPATH="$(cd -- "$(dirname "$0")" >/dev/null 2>&1 && pwd -P)"
PROJROOT="$(realpath "$SCRIPTPATH/..")"
UTILSPATH="$(realpath "$SCRIPTPATH/utils")"


DRY_RUN=""
SKIP_TEST=0

while [[ $# -gt 0 ]]; do
    case "$1" in
    --dry-run)
        DRY_RUN="--dry-run"
        shift
        ;;
    --skip-test)
        SKIP_TEST=1
        shift
        ;;
    *)
        echo "Unknown parameter: $1" >&2
        exit 1
        ;;
    esac
done

#
# check git working environment
#
# ensure we are inside a git repository
if ! git rev-parse --is-inside-work-tree &>/dev/null; then
    echo "error: Not inside a git repository" >&2
    exit 1
fi
# only allow releases of clean environment
if [[ -n $(git status --porcelain) ]]; then
    echo "error: There are uncommitted changes" >&2
    exit 1
fi
# Upstream check: only allow releases if branch is not ahead of its upstream
if git remote | grep -q .; then

    GIT_BRANCH=$(git symbolic-ref --short HEAD)

    if git rev-parse --abbrev-ref "$GIT_BRANCH@{upstream}" &>/dev/null; then

        ahead=$(git rev-list --count "$GIT_BRANCH@{upstream}..HEAD")

        if [[ "$ahead" -gt 0 ]]; then
            echo "error: branch is ahead of its upstream by $ahead commits. Please push your changes first." >&2
            exit 1
        fi
    fi
fi

# get our version
VERSION="$("$UTILSPATH/get-project-version.py" < "$PROJROOT/CMakeLists.txt")"
GIT_TAG="v$VERSION"
echo "Releasing tcppump version: $VERSION (git tag: $GIT_TAG)"


# check if tag already exists
if [ $(git tag -l "$GIT_TAG") ]; then
    echo "error: git tag $GIT_TAG already existing" >&2
    exit 1
fi

#
# before we create the release, we build and test everything
#
echo "-- build and test"
if (( SKIP_TEST )); then
  echo "skipped"
else
    # trigger github workflow to build and test
    echo "   starting github workflow"
    RUN_ID=$($UTILSPATH/trigger-github-workflow.sh --workflow .github/workflows/build-and-test.yml --ref "$GIT_BRANCH")

    # run build and test inside all docker containers
    echo "   running inside docker containers"
    $SCRIPTPATH/build-and-test-inside-container.sh > /dev/null

    # run build and test with address sanitizer and memcheck
    echo "   running local ASAN test"
    $SCRIPTPATH/build-and-test-GCC-with-ASAN.sh > /dev/null
    echo "   running local valgrind test"
    $SCRIPTPATH/build-and-test-GCC-with-memcheck.sh > /dev/null

    # await github workflow result
    echo "   awaiting github workflow result"
    $UTILSPATH/await-github-workflow-result.sh --id "$RUN_ID"
fi

#
# update changelog
#
echo "-- set changelog to 'released'"
echo $($UTILSPATH/release-changelog.py $DRY_RUN "$PROJROOT/CHANGELOG.md")

#
# git: commit and tag our release
#
echo "-- commit and tag release"
echo $GIT_TAG
if [[ -z "$DRY_RUN" ]]; then
    git commit $DRY_RUN -m "tcppump release $VERSION" "$PROJROOT/CHANGELOG.md"
    git tag -s -a $GIT_TAG -m "tcppump release $VERSION"

    git push origin HEAD tag $GIT_TAG
fi

#
# finally create source-tarball and release packages
#
echo "-- create packages"
if [[ -z "$DRY_RUN" ]]; then
    GIT_REF="$GIT_TAG"
else
    GIT_REF="$GIT_BRANCH"
fi
# trigger github workflow to build windows release binary
RUN_ID=$($UTILSPATH/trigger-github-workflow.sh --workflow .github/workflows/build-release-binary-windows.yml --ref "$GIT_REF")
# use the time to locally create deb and rpm packages as well
$SCRIPTPATH/package.sh debian.oldstable fedora.latest
# await github workflow result and download artifacts
WIN_RELEASE_DIR="$PROJROOT/RELEASE/windows"
$UTILSPATH/await-github-workflow-result.sh --id "$RUN_ID" --out "$WIN_RELEASE_DIR"


#
# windows artifacts build on github need some additional checks and preparation
#
echo "-- check and prepare windows release artifacts"
# verify windows executable
unzip -q "$WIN_RELEASE_DIR/release-windows-x64/tcppump-windows-x64.zip" -d "$WIN_RELEASE_DIR"
EXE="$WIN_RELEASE_DIR/tcppump.exe"
HASHFILE="$WIN_RELEASE_DIR/tcppump.exe.sha256"
FINAL_WIN_ZIP="$WIN_RELEASE_DIR/tcppump-$VERSION-windows-x64.zip"

if [[ ! -f "$EXE" || ! -f "$HASHFILE" ]]; then
  echo "Error: executable or hashfile not found!" >&2
  exit 1
fi

EXPECTED_HASH=$(tr -d '\r\n' < "$HASHFILE")
CALCULATED_HASH=$(sha256sum "$EXE" | awk '{print toupper($1)}')

if [[ "$EXPECTED_HASH" != "$CALCULATED_HASH" ]]; then
  echo "Error: hash mismatch for downloaded windows executable!" >&2
  exit 2
fi

rm "$EXE" "$HASHFILE"

mv "$WIN_RELEASE_DIR/release-windows-x64/tcppump-windows-x64.zip" "$FINAL_WIN_ZIP"
rm -r "$WIN_RELEASE_DIR/release-windows-x64"
# everything looks good, sign and create checksum for final windows zip
cd "$WIN_RELEASE_DIR"
gpg --verbose --detach-sig --yes -a "$FINAL_WIN_ZIP"
sha256sum "$FINAL_WIN_ZIP" | tee "$FINAL_WIN_ZIP.sha256"


echo "-- that's all folks, $VERSION released --"
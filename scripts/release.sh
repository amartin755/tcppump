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
# now let's do a test build
#
echo "-- build and test"
if (( SKIP_TEST )); then
  echo "skipped"
else
    # trigger github workflow to build and test
    RUN_ID=$($UTILSPATH/trigger-github-workflow.sh --workflow .github/workflows/build-and-test.yml --ref "$GIT_BRANCH")
    # run build and test inside all docker containers as well
    $SCRIPTPATH/build-and-test-inside-container.sh > /dev/null
    # await github workflow result
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

    git push && git push origin tag $GIT_TAG
fi

#
# finally create source-tarball and release packages
#
echo "-- create packages"
# trigger github workflow to build windows release binary
RUN_ID=$($UTILSPATH/trigger-github-workflow.sh --workflow .github/workflows/build-release-binary-windows.yml --ref "$GIT_TAG")
# use the time to locally create deb and rpm packages as well
$SCRIPTPATH/package.sh debian.oldstable fedora.latest
# await github workflow result and download artifacts
$UTILSPATH/await-github-workflow-result.sh --id "$RUN_ID" --out "$PROJROOT/RELEASE/windows"

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

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
PROJROOT=$(realpath $SCRIPTPATH/../..)

WORKFLOW=".github/workflows/build-release-binary-windows.yml"
REF="master"


usage() {
  cat <<EOF
Usage: $0 [--workflow PATH] [--ref REF]

Defaults:
  --workflow $WORKFLOW
  --ref $REF

Requires: gh, jq
EOF
  exit 1
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --workflow) WORKFLOW="$2"; shift 2;;
        --ref) REF="$2"; shift 2;;
        -h|--help) usage;;
        *) echo "Unknown arg: $1" >&2; usage;;
    esac
done

command -v gh >/dev/null 2>&1 || { echo "error: gh (GitHub CLI) required" >&2; exit 2; }
command -v jq >/dev/null 2>&1 || { echo "error: jq required" >&2; exit 2; }

# Require GH_TOKEN to be set for non-interactive runs
if [[ -z "${GH_TOKEN:-}" ]]; then
  echo "error: GH_TOKEN is not set. Run 'gh auth login' or export GH_TOKEN with a PAT." >&2
  exit 3
fi

#echo "Triggering workflow: $WORKFLOW (ref: $REF)"
gh workflow run "$WORKFLOW" --ref "$REF" > /dev/null

#echo "Waiting a few seconds for GitHub to register the run..."
sleep 3

#echo "Locating latest run databaseId for workflow $WORKFLOW"
RUN_ID=$(gh run list --workflow "$WORKFLOW" --limit 20 --json databaseId,createdAt | jq -r 'sort_by(.createdAt) | last.databaseId')

if [[ -z "$RUN_ID" || "$RUN_ID" == "null" ]]; then
    echo "error: could not find a workflow run id" >&2
    exit 3
fi

echo "$RUN_ID"

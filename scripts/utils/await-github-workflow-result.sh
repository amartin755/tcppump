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


OUTDIR=""
TIMEOUT=1800
POLL=10

usage() {
  cat <<EOF
Usage: $0 [--out DIR] [--timeout SEC] [--poll SEC] --id RUN_ID

Defaults:
  --timeout $TIMEOUT
  --poll $POLL

Requires: gh, jq
EOF
  exit 1
}

while [[ $# -gt 0 ]]; do
    case "$1" in
      --id) RUN_ID="$2"; shift 2;;
      --out) OUTDIR="$2"; shift 2;;
      --timeout) TIMEOUT="$2"; shift 2;;
      --poll) POLL="$2"; shift 2;;
      -h|--help) usage;;
      *) echo "Unknown arg: $1" >&2; usage;;
    esac
done
if [[ -z "${RUN_ID:-}" ]]; then
    echo "error: --id RUN_ID is required" >&2
    usage
fi

command -v gh >/dev/null 2>&1 || { echo "error: gh (GitHub CLI) required" >&2; exit 2; }
command -v jq >/dev/null 2>&1 || { echo "error: jq required" >&2; exit 2; }

# Require GH_TOKEN to be set for non-interactive runs
if [[ -z "${GH_TOKEN:-}" ]]; then
    echo "error: GH_TOKEN is not set. Run 'gh auth login' or export GH_TOKEN with a PAT." >&2
    exit 3
fi

START=$(date +%s)
while true; do
    status=$(gh run view "$RUN_ID" --json status,conclusion | jq -r '.status')
    echo "status: $status"
    if [[ "$status" == "completed" ]]; then
        conclusion=$(gh run view "$RUN_ID" --json status,conclusion | jq -r '.conclusion')
        echo "Run completed: $conclusion"
        break
    fi
    NOW=$(date +%s)
    elapsed=$((NOW-START))
    if (( elapsed > TIMEOUT )); then
        echo "error: timeout waiting for workflow (>$TIMEOUT s)" >&2
        exit 4
    fi
    sleep "$POLL"
done

if [[ -n "$OUTDIR" ]]; then
    echo "Downloading artifacts to $OUTDIR"
    mkdir -p "$OUTDIR"
    gh run download "$RUN_ID" --dir "$OUTDIR"
    echo "Done. Artifacts saved in: $OUTDIR"
fi

if [[ -n "${conclusion:-}" && "$conclusion" != "success" ]]; then
    echo "warning: workflow finished with conclusion: $conclusion" >&2
    exit 5
fi

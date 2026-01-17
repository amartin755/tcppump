#!/usr/bin/env python3
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

import sys
import re
import argparse
import tempfile
import os
from pathlib import Path
from datetime import datetime, timezone


def die(msg: str, code: int = 1):
    print(f"ERROR: {msg}", file=sys.stderr)
    sys.exit(code)


def get_release_time():
    """
    Returns:
      (datetime_obj, epoch_seconds_int)
    """
    sde = os.getenv("SOURCE_DATE_EPOCH")

    if sde is not None:
        try:
            epoch = int(sde)
        except ValueError:
            die("SOURCE_DATE_EPOCH is not a valid integer")

        dt = datetime.fromtimestamp(epoch, tz=timezone.utc)
        return dt, epoch

    # fallback: current time
    dt = datetime.now().astimezone()
    epoch = int(dt.timestamp())
    return dt, epoch


def main():
    parser = argparse.ArgumentParser(
        description="Replace _UNRELEASED_ in changelog with release timestamp"
    )
    parser.add_argument(
        "file",
        help="Path to changelog file"
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Do not modify file, only print resulting timestamp"
    )
    args = parser.parse_args()

    changelog = Path(args.file)

    if not changelog.exists():
        die(f"File not found: {changelog}")

    try:
        content = changelog.read_text(encoding="utf-8")
    except Exception as e:
        die(f"Unable to read file: {e}")

    if "_UNRELEASED_" not in content:
        die("No _UNRELEASED_ entry found")

    dt, epoch = get_release_time()

    # Format for changelog (human readable)
    human_ts = dt.strftime("%Y-%m-%d %H:%M:%S %z")

    try:
        new_content, count = re.subn(
            r"_UNRELEASED_",
            f"_{human_ts}_",
            content,
            count=1
        )
    except Exception as e:
        die(f"Regex replace failed: {e}")

    if count != 1:
        die("Replacement failed (unexpected match count)")

    # DRY RUN → no file modifications
    if args.dry_run:
        print(epoch)
        return

    # Atomic write
    try:
        with tempfile.NamedTemporaryFile(
            "w",
            encoding="utf-8",
            dir=changelog.parent,
            delete=False
        ) as tmp:
            tmp.write(new_content)
            tmp_path = Path(tmp.name)

        tmp_path.replace(changelog)

    except Exception as e:
        die(f"Atomic write failed: {e}")

    # OUTPUT: SOURCE_DATE_EPOCH format (unix epoch)
    print(epoch)


if __name__ == "__main__":
    main()

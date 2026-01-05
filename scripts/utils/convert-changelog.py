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
from datetime import datetime, timezone


def parse_date(date_str):
    for fmt in ("%Y-%m-%d %H:%M:%S %z", "%Y-%m-%d %H:%M:%S", "%Y-%m-%d"):
        try:
            return datetime.strptime(date_str, fmt)
        except ValueError:
            continue
    raise ValueError(f"Unsupported date format: {date_str}")


def parse_markdown(lines):
    changelog = []
    current_version = None
    current_date = None
    current_entries = []
    current_category = None
    current_maintainer = None
    current_email = None
    seen_category = False
    version_start_line = None

    def finalize_version():
        if not current_version:
            return None
        if not current_date:
            raise ValueError(f"Line {version_start_line}: Missing date line for version {current_version}")
        if not current_maintainer or not current_email:
            raise ValueError(f"Line {version_start_line}: Missing maintainer/email for version {current_version}")
        if not seen_category:
            raise ValueError(f"Line {version_start_line}: Missing category for version {current_version}")
        if not current_entries:
            raise ValueError(f"Line {version_start_line}: No entries for version {current_version}")
        return (current_version, current_date, current_entries, current_maintainer, current_email)

    for idx, line in enumerate(lines, start=1):
        line = line.rstrip()

        # version header
        m = re.match(r"^# (.+)$", line)
        if m:
            if current_version:
                entry = finalize_version()
                if entry:
                    changelog.append(entry)
                current_entries = []
                seen_category = False

            current_version = m.group(1).strip()
            current_date = None
            current_maintainer = None
            current_email = None
            version_start_line = idx
            continue

        # release date, maintainer, email
        m = re.match(r"^_([^_]+)_\s*,\s*(.+)$", line)
        if m and current_version:
            date_str = m.group(1).strip()
            maintainer_info = m.group(2)

            m2 = re.match(r"(.+?)\s*<([^>]+)>", maintainer_info)
            if not m2:
                raise ValueError(f"Line {idx}: Invalid maintainer/email format in line: {line}")
            current_maintainer = m2.group(1).strip()
            current_email = m2.group(2).strip()

            if date_str == "UNRELEASED":
                current_date = ("UNRELEASED", datetime.now(timezone.utc))
            else:
                current_date = ("RELEASED", parse_date(date_str))
            continue

        # category
        m = re.match(r"^## (.+)$", line)
        if m:
            current_category = m.group(1).strip()
            seen_category = True
            continue

        # changelog item
        m = re.match(r"^- (.+)$", line)
        if m and current_category:
            current_entries.append((current_category, m.group(1).strip()))
            continue

    if current_version:
        entry = finalize_version()
        if entry:
            changelog.append(entry)

    return changelog


def to_debian(changelog, package_name):
    output = []
    for version, (status, date), entries, maintainer, email in changelog:
        distribution = "UNRELEASED" if status == "UNRELEASED" else "unstable"
        date_str = (
            datetime.now(timezone.utc).strftime("%a, %d %b %Y %H:%M:%S +0000")
            if status == "UNRELEASED"
            else date.strftime("%a, %d %b %Y %H:%M:%S %z")
        )

        output.append(f"{package_name} ({version}-1) {distribution}; urgency=medium\n")

        categories = {}
        for cat, entry in entries:
            categories.setdefault(cat, []).append(entry)

        for cat, cat_entries in categories.items():
            output.append(f"  * {cat}\n")
            for e in cat_entries:
                output.append(f"    - {e}\n")

        output.append(f"\n -- {maintainer} <{email}>  {date_str}\n\n")

    return "".join(output)


def to_rpm(changelog, package_name):
    output = ["%changelog\n"]
    for version, (status, date), entries, maintainer, email in changelog:
        date_str = (
            datetime.now(timezone.utc).strftime("%a %b %d %Y")
            if status == "UNRELEASED"
            else date.strftime("%a %b %d %Y")
        )
        output.append(f"* {date_str} {maintainer} <{email}> - {version}-1\n")

        categories = {}
        for cat, entry in entries:
            categories.setdefault(cat, []).append(entry)

        for cat, cat_entries in categories.items():
            output.append(f"- {cat}\n")
            for e in cat_entries:
                output.append(f"  * {e}\n")

        output.append("\n")

    return "".join(output)


def main():
    parser = argparse.ArgumentParser(description="Convert CHANGELOG.md to Debian or RPM format.")
    parser.add_argument("--package", required=True, help="Package name")
    parser.add_argument("--format", choices=["debian", "rpm"], default="debian", help="Output format")
    args = parser.parse_args()

    lines = sys.stdin.readlines()
    try:
        changelog = parse_markdown(lines)
    except ValueError as e:
        sys.stderr.write(f"Error: {e}\n")
        sys.exit(1)

    if args.format == "debian":
        output = to_debian(changelog, args.package)
    else:
        output = to_rpm(changelog, args.package)

    sys.stdout.write(output)


if __name__ == "__main__":
    main()

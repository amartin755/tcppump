#!/usr/bin/env python3

import re
import sys

content = sys.stdin.read()

match = re.search(
    r'project\s*\([^\)]*?VERSION\s+([0-9]+\.[0-9]+\.[0-9]+)',
    content,
    re.DOTALL | re.IGNORECASE
)

if match:
    version = match.group(1)
    print(version)
else:
    exit (1)

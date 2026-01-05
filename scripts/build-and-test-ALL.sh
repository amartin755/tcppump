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

set -e

BUILDDIRNAME=build
SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
PROJROOT=$(realpath $SCRIPTPATH/..)

cd $PROJROOT

$SCRIPTPATH/build-and-test-GCC.sh
$SCRIPTPATH/build-and-test-GCC-with-ASAN.sh
$SCRIPTPATH/build-and-test-GCC-with-UBSAN.sh
$SCRIPTPATH/build-and-test-GCC-with-memcheck.sh

$SCRIPTPATH/build-and-test-CLANG.sh
$SCRIPTPATH/build-and-test-CLANG-with-ASAN.sh
$SCRIPTPATH/build-and-test-CLANG-with-UBSAN.sh
$SCRIPTPATH/build-and-test-CLANG-with-memcheck.sh

echo "----- SUCCESS ----"

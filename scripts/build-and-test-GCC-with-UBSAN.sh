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

BUILDDIRNAME="${1:-build}"
SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
PROJROOT=$(realpath $SCRIPTPATH/..)



# clean configure and build
###############################################################################
$SCRIPTPATH/utils/clean-build.sh -t -u -C RelWithDebInfo -T GCC -B $BUILDDIRNAME


# run all ctest cases
###############################################################################

# set raw capabilities
echo "NOTE: need sudo for setting of raw-capabilities for tcppump binary"
cmake --build $BUILDDIRNAME --target setcap

# execute ctest
cd $BUILDDIRNAME
ctest

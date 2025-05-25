# SPDX-License-Identifier: GPL-3.0-only
###############################################################################
#
# TCPPUMP <https://github.com/amartin755/tcppump>
# Copyright (C) 2012-2025 Andreas Martin (netnag@mailbox.org)
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



# Function: HAS_FILE_PREFIX
# Purpose:
#   Checks if the input string starts with the "file://" prefix.
#   If it does, the prefix is removed and a flag is set to TRUE.
#   Otherwise, the original string is returned and the flag is FALSE.
#
# Parameters:
#   input_string   - The input string to examine.
#   output_string  - The name of the output variable to store the resulting string.
#   was_prefixed   - The name of the output variable to indicate if the prefix was present.
function (HAS_FILE_PREFIX input_string output_string was_prefixed)
    # Check if the string starts with "file://"
    string (FIND "${input_string}" "file://" prefix_pos)

    if(prefix_pos EQUAL 0)
        # Remove the prefix
        string(LENGTH "file://" prefix_length)
        string(SUBSTRING "${input_string}" ${prefix_length} -1 cleaned_string)
        set(${output_string} "${cleaned_string}" PARENT_SCOPE)
        set(${was_prefixed} TRUE PARENT_SCOPE)
    else()
        # No prefix found, return original string
        set(${output_string} "${input_string}" PARENT_SCOPE)
        set(${was_prefixed} FALSE PARENT_SCOPE)
    endif()
endfunction()

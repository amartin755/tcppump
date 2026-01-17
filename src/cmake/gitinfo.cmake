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

function (git_info)

execute_process (COMMAND git rev-parse --short HEAD    OUTPUT_VARIABLE GIT_COMMIT_SHORT ERROR_QUIET)

if (NOT "${GIT_COMMIT_SHORT}" STREQUAL "")
    execute_process (COMMAND git describe --exact-match --tags OUTPUT_VARIABLE GIT_TAG ERROR_QUIET)
    execute_process (COMMAND git rev-parse --abbrev-ref HEAD OUTPUT_VARIABLE GIT_BRANCH)
    execute_process (COMMAND git rev-parse HEAD OUTPUT_VARIABLE GIT_COMMIT)
    execute_process(
        COMMAND git diff --quiet --exit-code
        RESULT_VARIABLE GIT_HAS_LOCAL_CHANGES
        )

    if (${GIT_HAS_LOCAL_CHANGES} EQUAL 1)
        set (GIT_HAS_LOCAL_CHANGES "(modified)")
    endif ()

    string (STRIP "${GIT_COMMIT_SHORT}" GIT_COMMIT_SHORT)
    string (STRIP "${GIT_COMMIT}" GIT_COMMIT)
    string (STRIP "${GIT_TAG}" GIT_TAG)
    string (STRIP "${GIT_BRANCH}" GIT_BRANCH)

    set (GIT_COMMIT ${GIT_COMMIT}${GIT_HAS_LOCAL_CHANGES} PARENT_SCOPE)
    set (GIT_TAG "${GIT_TAG}" PARENT_SCOPE)
    set (GIT_BRANCH "${GIT_BRANCH}" PARENT_SCOPE)
else ()
    if(EXISTS "${CMAKE_SOURCE_DIR}/VERSION")

        file(READ "${CMAKE_SOURCE_DIR}/VERSION" _content)

        string(REGEX MATCH "COMMIT[ \t]+([^\n\r]+)" _ ${_content})
        set(COMMIT_VALUE "${CMAKE_MATCH_1}")

        string(REGEX MATCH "TAG[ \t]+([^\n\r]+)" _ ${_content})
        set(TAG_VALUE "${CMAKE_MATCH_1}")

        set (GIT_COMMIT ${COMMIT_VALUE} PARENT_SCOPE)
        set (GIT_TAG "${TAG_VALUE}" PARENT_SCOPE)
    endif()
endif()

endfunction()


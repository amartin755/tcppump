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


###############################################################################
# tcppump tests
#
# Note: Most of the tests are defined in testcases.yaml which are converted to 
# cmake by a python scripts (see below)
#
# ctest logic
# - test fails if executable returns != 0
# - if property WILL_FAIL is set test failes if return == 0
# - if property PASS_REGULAR_EXPRESSION is used, the return code is IGNORED!
#   The compare result of the stdout/stderr output defines the test-success
#   --> fails if output != PASS_REGULAR_EXPRESSION
# - property WILL_FAIL combined with PASS_REGULAR_EXPRESSION:
#   --> failes, if output == PASS_REGULAR_EXPRESSION
###############################################################################


if (WIN32)
    set (OUT_IFC "Ethernet")
endif ()
if (UNIX)
    set (OUT_IFC lo)
endif ()
if (TEST_NET_ADAPTER)
    set (OUT_IFC ${TEST_NET_ADAPTER})
endif ()
message(STATUS "Using network adapter '${OUT_IFC}' for ctest")

# temp dir where files are stored
set (TEST_TMP_DIR ${CMAKE_BINARY_DIR}/ctest_tmp)
# directory of reference pcap file
set (REF_FILES_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Create setup testcases
###############################################################################
# remove temp directory
add_test (NAME setup-1 COMMAND cmake -E rm -rf ${TEST_TMP_DIR})
set_tests_properties (setup-1 PROPERTIES FIXTURES_SETUP RM_TMP)
# create new temp dir
add_test (NAME setup-2 COMMAND cmake -E make_directory -rf ${TEST_TMP_DIR})
set_tests_properties (setup-2 PROPERTIES FIXTURES_REQUIRED RM_TMP)
set_tests_properties (setup-2 PROPERTIES FIXTURES_SETUP setup)

# additional test cases
###############################################################################
if (WITH_UNITTESTS)
    add_test(NAME unittests COMMAND unittest ${REF_FILES_DIR})
endif()

add_test(NAME show-version-output--ok COMMAND tcppump --version)
set_tests_properties(show-version-output--ok PROPERTIES PASS_REGULAR_EXPRESSION "tcppump ${PROJECT_VERSION}")

# Load test cases from YAML
###############################################################################
find_package(Python3 COMPONENTS Interpreter)
# in case python is not available, tests are possible, but can't be updated
if (Python3_FOUND)
    # command to convert yaml defined testcases to cmake testcases
    add_custom_command (
        OUTPUT ${CMAKE_CURRENT_LIST_DIR}/testcases.cmake
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/generate_testcases.py
                ${CMAKE_CURRENT_LIST_DIR}/testcases.yaml
                ${CMAKE_CURRENT_LIST_DIR}/testcases.cmake
        DEPENDS ${CMAKE_CURRENT_LIST_DIR}/testcases.yaml
    )
    # execute this target, if testcases.yaml has been changed
    add_custom_target (generate_tests
        DEPENDS ${CMAKE_CURRENT_LIST_DIR}/testcases.cmake
)
endif ()

# include generated testcases
include (${CMAKE_CURRENT_LIST_DIR}/testcases.cmake)

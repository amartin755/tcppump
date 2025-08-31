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


###############################################################################
# tcppump tests
###############################################################################

# ctest logic
# - test fails if executable returns != 0
# - if property WILL_FAIL is set test failes if return == 0
# - if property PASS_REGULAR_EXPRESSION is used, the return code is IGNORED!
#   The compare result of the stdout/stderr output defines the test-success
#   --> fails if output != PASS_REGULAR_EXPRESSION
# - property WILL_FAIL combined with PASS_REGULAR_EXPRESSION:
#   --> failes, if output == PASS_REGULAR_EXPRESSION

if(CMAKE_VERSION VERSION_LESS "3.19") 
    message (WARNING "At least cmake version 3.19 needed for ctest (${CMAKE_VERSION}). Build without test is still possible.")
else ()

if (WIN32)
    set (OUT_IFC "Ethernet") #FIXME we need a reliable way to find a suitable test interface in windows
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


# Load test cases from JSON
###############################################################################
file (READ "${CMAKE_CURRENT_LIST_DIR}/testcases.json" json_file)

# extract default commandline parameter -> DEFAULT_PARAMS
string (JSON json_defaults GET "${json_file}" defaults)
string (JSON ARRAY_COUNT LENGTH "${json_defaults}")
math (EXPR ARRAY_LAST_INDEX "${ARRAY_COUNT} - 1")
foreach (i RANGE 0 ${ARRAY_LAST_INDEX})
    string (JSON json_array_element GET "${json_defaults}" ${i})
    list (APPEND DEFAULT_PARAMS ${json_array_element})
endforeach ()

# extract testcases array
string (JSON json_testcases GET "${json_file}" testcases)
string (JSON TEST_COUNT LENGTH "${json_testcases}")
math (EXPR LAST_INDEX "${TEST_COUNT} - 1")

# iterate over all testcases
foreach (test_idx RANGE 0 ${LAST_INDEX})

    # testcase name -> json_test_name
    string (JSON json_test_name  GET "${json_testcases}" ${test_idx} name)

    # input parameters as array (optional) -> list IN_PARAMS
    string (JSON json_test_inputs ERROR_VARIABLE NOT_FOUND GET "${json_testcases}" ${test_idx} input)
    if (NOT NOT_FOUND)
        string (JSON ARRAY_COUNT LENGTH "${json_test_inputs}")
        unset (IN_PARAMS)
        math (EXPR ARRAY_LAST_INDEX "${ARRAY_COUNT} - 1")

        foreach (i RANGE 0 ${ARRAY_LAST_INDEX})
            string (JSON json_array_element GET "${json_testcases}" ${test_idx} input ${i})
            has_file_prefix("${json_array_element}" input_par prefix_found)
            if (prefix_found)
                list (APPEND IN_PARAMS ${REF_FILES_DIR}/${input_par})
            else ()
                list (APPEND IN_PARAMS ${input_par})
            endif ()
        endforeach ()
    endif ()

    # expected to fail? -> json_will_fail
    string (JSON json_will_fail ERROR_VARIABLE NOT_FOUND GET "${json_testcases}" ${test_idx} will_fail)
    if (NOT_FOUND)
        set (json_will_fail "OFF")
        unset (NOT_FOUND)
    endif ()

    # if testcase overwrites "defaults" use them instead -> TEST_PARAMS
    # if not TEST_PARAMS = DEFAULT_PARAMS
    string (JSON json_defaults ERROR_VARIABLE NOT_FOUND GET "${json_testcases}" ${test_idx} defaults)
    if (NOT_FOUND)
        set (TEST_PARAMS ${DEFAULT_PARAMS})
        unset (NOT_FOUND)
    else ()
        set (TEST_PARAMS "")
        string (JSON ARRAY_COUNT LENGTH "${json_defaults}")
        if (NOT ARRAY_COUNT EQUAL 0)
            math (EXPR ARRAY_LAST_INDEX "${ARRAY_COUNT} - 1")
            foreach (i RANGE 0 ${ARRAY_LAST_INDEX})
                string (JSON json_array_element GET "${json_testcases}" ${test_idx} defaults ${i})
                list (APPEND TEST_PARAMS ${json_array_element})
            endforeach ()
        endif ()
    endif ()

    # send to network? -> add -i parameter to TEST_PARAMS
    string (JSON json_live ERROR_VARIABLE NOT_FOUND GET "${json_testcases}" ${test_idx} live)
    if (NOT_FOUND)
        set (json_live "OFF")
        unset (NOT_FOUND)
    else ()
        list (APPEND TEST_PARAMS -i ${OUT_IFC})
    endif ()

    # expected output -> json_expected_output
    # if !live, set OUT_PARAMS to pcap (file provided) or hexstream
    string (JSON json_expected_output ERROR_VARIABLE NOT_FOUND GET "${json_testcases}" ${test_idx} expected_output)
    unset (PCAP_FILE)
    unset (OUT_PARAMS)
    if (NOT_FOUND)
        unset (NOT_FOUND)
        unset (json_expected_output)
    else ()
        if (json_live STREQUAL "OFF")
            has_file_prefix("${json_expected_output}" PCAP_FILE prefix_found)
            if (prefix_found)
                set (OUT_PARAMS -F pcap -w "${TEST_TMP_DIR}/${json_test_name}.pcap")
            else ()
                unset (PCAP_FILE)
                set (OUT_PARAMS -F hexstream -w -)
            endif ()
        endif ()
    endif ()

    # add additional commandline parameters, if provided -> TEST_PARAMS
    string (JSON json_options ERROR_VARIABLE NOT_FOUND GET "${json_testcases}" ${test_idx} options)
    if (NOT_FOUND)
        unset (NOT_FOUND)
    else ()
        string (JSON ARRAY_COUNT LENGTH "${json_options}")
        math (EXPR ARRAY_LAST_INDEX "${ARRAY_COUNT} - 1")
        foreach (i RANGE 0 ${ARRAY_LAST_INDEX})
            string(JSON json_array_element GET "${json_testcases}" ${test_idx} options ${i})
            list (APPEND TEST_PARAMS ${json_array_element})
        endforeach ()
    endif ()

    # create testcase with a dependency to the setup testcase
    add_test (
        NAME "${json_test_name}"
        COMMAND tcppump ${TEST_PARAMS} ${OUT_PARAMS} ${IN_PARAMS}
    )
    set_tests_properties ("${json_test_name}" PROPERTIES FIXTURES_REQUIRED setup)

    # if output should be compared to a existing pcap file, we create an additional diff testcase
    # Othewise the text output is compared to the expected string (if provided)
    if (PCAP_FILE)
        add_test (NAME "${json_test_name}-diff" COMMAND cmake -E compare_files "${TEST_TMP_DIR}/${json_test_name}.pcap" "${REF_FILES_DIR}/${PCAP_FILE}")
        # diff testcase must be run after the real testcase
        set_tests_properties ("${json_test_name}" PROPERTIES FIXTURES_SETUP  "${json_test_name}-setup")
        set_tests_properties ("${json_test_name}-diff" PROPERTIES FIXTURES_REQUIRED "${json_test_name}-setup")
    else ()
        if (json_expected_output)
            set_tests_properties ("${json_test_name}" PROPERTIES PASS_REGULAR_EXPRESSION "${json_expected_output}")
        endif ()
    endif ()
    if (json_will_fail STREQUAL "ON")
        # test is expected to fail
        set_tests_properties ("${json_test_name}" PROPERTIES WILL_FAIL TRUE)
    endif ()
endforeach ()


# additional test cases
###############################################################################
if (WITH_UNITTESTS)
    #FIXME unittests use hardcoded relative paths (e.g. ../src/test/readtest.pcap)
    add_test(NAME unittests COMMAND unittest WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

add_test(NAME show-version-output--ok COMMAND tcppump --version)
set_tests_properties(show-version-output--ok PROPERTIES PASS_REGULAR_EXPRESSION "tcppump ${PROJECT_VERSION}")

endif ()

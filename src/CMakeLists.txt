###############################################################################
#
# TCPPUMP <https://github.com/amartin755/tcppump>
# Copyright (C) 2019-2020 Andreas Martin (netnag@mailbox.org)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
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
cmake_minimum_required (VERSION 3.13)

project (tcppump VERSION 0.1.0
                 DESCRIPTION "Ethernet packet generator"
                 LANGUAGES CXX)
###############################################################################


# set output directories to "bin"
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/../bin)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/../bin)


# compiler settings
###############################################################################
# C++11 is required
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# warning level
if (MSVC)
    add_compile_options (/W3)
else()
    add_compile_options (-Wall -Wextra)
endif()

# on mingw we link the std library statically
if (MINGW)
    add_link_options (-static-libgcc -static-libstdc++)
endif ()

# check dependencies
###############################################################################
include (CheckLibraryExists)
include (CheckSymbolExists)
include (CheckFunctionExists)
include (CheckIncludeFiles)

# try to find pcap library; can be set via PCAP_INCLUDE_DIR and PCAP_LIBRARY
if (NOT PCAP_INCLUDE_DIR)
    set (PCAPDIR ${PROJECT_SOURCE_DIR}/../pcap/include)
    find_path (PCAP_INCLUDE_DIR NAMES pcap/pcap.h pcap.h PATHS ${PCAPDIR})
endif ()
if (NOT PCAP_LIBRARY)
    set (PCAPDIR ${PROJECT_SOURCE_DIR}/../pcap/lib)
else ()
    set (PCAPDIR ${PCAP_LIBRARY})
    unset (PCAP_LIBRARY CACHE)
endif ()
find_library (__PCAP_LIBRARY NAMES wpcap pcap PATHS ${PCAPDIR})
message (STATUS "PCAP_INCLUDE_DIR = ${PCAP_INCLUDE_DIR}")
message (STATUS "PCAP_LIBRARY     = ${__PCAP_LIBRARY}")
set (CMAKE_REQUIRED_LIBRARIES ${__PCAP_LIBRARY})
set (CMAKE_REQUIRED_INCLUDES ${PCAP_INCLUDE_DIR})
check_symbol_exists (pcap_open_offline "stdio.h;pcap.h" HAVE_PCAP)
set (CMAKE_REQUIRED_LIBRARIES "")
set (CMAKE_REQUIRED_INCLUDES "")


if (WIN32)
    # winsock, winpcap, conio and iphlpapi are needed on windows
    check_include_files("winsock2.h;iphlpapi.h;winerror.h" HAVE_WS2)
    # getch() is needed for unbuffered stdin on Windows
    check_symbol_exists (getch "conio.h" HAVE_GETCH)
    # check for inet_pton() inet_ntop(), they do not exist on older mingw
    set (CMAKE_REQUIRED_LIBRARIES "ws2_32")
    set (CMAKE_REQUIRED_DEFINITIONS -D_WIN32_WINNT=0x0600)
    check_symbol_exists (inet_pton "ws2tcpip.h" HAVE_PTON)
    check_symbol_exists (inet_ntop "ws2tcpip.h" HAVE_NTOP)
    set (CMAKE_REQUIRED_LIBRARIES "")
    set (CMAKE_REQUIRED_DEFINITIONS "")

    if (NOT HAVE_GETCH)
        message (FATAL_ERROR "Function getch() not found!")
    endif ()
    if (NOT HAVE_WS2)
        message (FATAL_ERROR "Winsock and ip helper headers not found!")
    endif ()
    if (NOT HAVE_PCAP)
        message (FATAL_ERROR "Library wpcap not found!")
    endif ()
else ()
    check_symbol_exists (inet_pton "arpa/inet.h" HAVE_PTON)
    check_symbol_exists (inet_ntop "arpa/inet.h" HAVE_NTOP)
endif ()

check_include_files("sys/time.h" HAVE_SYSTIME_H)
check_include_files("arpa/inet.h" HAVE_ARPAINET_H)


# preprocessor definitions
###############################################################################
add_compile_definitions (APP_VERSION="${PROJECT_VERSION}")

if (HAVE_WS2)
    add_compile_definitions (HAVE_WS2)
endif ()
if (HAVE_SYSTIME_H)
    add_compile_definitions (HAVE_SYSTIME_H)
endif ()
if (HAVE_ARPAINET_H)
    add_compile_definitions (HAVE_ARPAINET_H)
endif ()
if (HAVE_PCAP)
    add_compile_definitions (HAVE_PCAP)
endif ()
if (HAVE_PTON)
    add_compile_definitions (HAVE_PTON)
endif ()
if (HAVE_NTOP)
    add_compile_definitions (HAVE_NTOP)
endif ()

if (WIN32)
    add_compile_definitions (HAVE_WINDOWS)
    # min. supported version is Windows 7
    add_compile_definitions (_WIN32_WINNT=0x0601)
    add_compile_definitions (WINVER=0x0601)
    # only include minimum windows dependencies
    add_compile_definitions (WIN32_LEAN_AND_MEAN)
    # suppress warnings about unsafe strncpy and friends
    add_compile_definitions (_CRT_SECURE_NO_WARNINGS)
    # because of the workaround below we explicitly flag usage of ms visual c compiler
    if (MSVC)
        add_compile_definitions (HAVE_MSVC)
    endif ()

    #### workaround: pcap-stdinc.h does something like this:
    #  #if _MSC_VER < 1500
    #  #define snprintf _snprintf
    #  #define vsnprintf _vsnprintf
    #  #define strdup _strdup
    #  #endif
    # On non-ms compilers the if statement is always true and therefore can
    # lead to compile error with c++11 code
    if (NOT MSVC)
        if (HAVE_PCAP)
            add_compile_definitions (_MSC_VER=1500)
        endif ()
    endif ()

endif ()

# generate build numbers
###############################################################################
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
        set (GIT_HAS_LOCAL_CHANGES "+")
    endif ()

    string (STRIP "${GIT_COMMIT_SHORT}" GIT_COMMIT_SHORT)
    string (STRIP "${GIT_COMMIT}" GIT_COMMIT)
    string (STRIP "${GIT_TAG}" GIT_TAG)
    string (STRIP "${GIT_BRANCH}" GIT_BRANCH)

    set (GIT_COMMIT_SHORT ${GIT_COMMIT_SHORT}${GIT_HAS_LOCAL_CHANGES})
    set (GIT_COMMIT ${GIT_COMMIT}${GIT_HAS_LOCAL_CHANGES})

    add_compile_definitions (GIT_COMMIT_SHORT="${GIT_COMMIT_SHORT}")
    add_compile_definitions (GIT_TAG="${GIT_TAG}")
    add_compile_definitions (GIT_BRANCH="${GIT_BRANCH}")
    add_compile_definitions (GIT_COMMIT="${GIT_COMMIT}")
endif()

string(TIMESTAMP BUILD_TIME "%Y-%m-%d %H:%M" UTC)
add_compile_definitions(BUILD_TIME="${BUILD_TIME}")
add_compile_definitions (BUILD_TYPE="${CMAKE_BUILD_TYPE}")

message (STATUS "Build: type = '${CMAKE_BUILD_TYPE}' tag = '${GIT_TAG}' branch = '${GIT_BRANCH}' commit = '${GIT_COMMIT}'")

# source files
###############################################################################

# platform specific libs and files
if (WIN32)
    set (os_specific os/win32)
    set (libs
         ws2_32
         IPHLPAPI)
endif ()
if (UNIX)
    set (os_specific os/posix)
endif ()

# includes and libs
if (HAVE_PCAP)
    set (libs ${libs} ${__PCAP_LIBRARY})
    if (PCAP_INCLUDE_DIR)
        include_directories (${PCAP_INCLUDE_DIR})
    endif ()
endif ()


add_executable (tcppump)
add_subdirectory (common)
add_subdirectory (os)
add_subdirectory (packets)
add_subdirectory (parser)
add_subdirectory (pcap)

target_link_libraries (tcppump LINK_PUBLIC ${libs})
target_sources(tcppump PRIVATE tcppump.cpp ${SOURCES})
target_include_directories (tcppump PRIVATE ${INCLUDES})


if (UNIX)
    # add raw capablities in oder to be runable without root
    add_custom_command(TARGET tcppump
                       POST_BUILD
                       COMMAND sudo setcap cap_net_raw+eip ./tcppump
                       WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()


# unit test code
###############################################################################
if (WITH_UNITTESTS)

    add_executable (unittest)
    target_link_libraries (unittest PRIVATE ${libs})

    target_compile_definitions (unittest PRIVATE WITH_UNITTESTS)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options (unittest PRIVATE -Wno-write-strings --coverage)
        target_link_libraries (unittest PRIVATE gcov)
    endif()

    target_sources(unittest PRIVATE unittest.cpp ${SOURCES})
    target_include_directories (unittest PRIVATE ${INCLUDES})
endif ()


# tcppump tests
###############################################################################
enable_testing()

if (WIN32)
    set (OUT_IFC WiFi)
endif ()
if (UNIX)
    set (OUT_IFC lo)
endif ()
if (TEST_NET_ADAPTER)
    set (OUT_IFC ${TEST_NET_ADAPTER})
endif ()

message(STATUS "Using network adapter '${OUT_IFC}' for testing")

set (TEST_OUTPUT testoutput.pcap)
set (DEFAULT_PARAMS -i ${OUT_IFC} --myip4=1.2.3.4 --mymac=80:23:45:67:89:AB -o ${TEST_OUTPUT})
set (TEST_FILE_PREFIX ${CMAKE_BINARY_DIR}/../src/test)


add_test(NAME no-params--nok COMMAND tcppump WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(no-params--nok PROPERTIES WILL_FAIL TRUE)

add_test(NAME show-usage--ok COMMAND tcppump -h WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(NAME raw-1--ok COMMAND tcppump ${DEFAULT_PARAMS} -r "0123456789abcdef0123456789abcdef" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME raw-1--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/raw-01.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME raw-2--nok COMMAND tcppump ${DEFAULT_PARAMS} -r "abcdf" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(raw-2--nok PROPERTIES WILL_FAIL TRUE)

add_test(NAME raw-3--ok COMMAND tcppump ${DEFAULT_PARAMS} "raw(payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME raw-3--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/raw-03.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(NAME eth-1--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-1--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-01.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-2--nok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=1122:33:44:55:66, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(eth-2--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME eth-3--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, ethertype=0x9000, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-3--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-03.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-4--nok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, ethertype=0x10000, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(eth-4--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME eth-5--nok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(eth-5--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME eth-6--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, smac=80:81:82:83:84:85, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-6--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-06.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-7--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, vid=0, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-7--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-07.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-8--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, vid=4095, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-8--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-08.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-9--nok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, vid=4096, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(eth-9--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME eth-10--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, vid=4095, prio=0, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-10--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-08.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-11--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, vid=4095, prio=7, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-11--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-11.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-12--nok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, vid=4095, prio=8, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(eth-12--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME eth-13--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, vid=4095, prio=7, dei=0, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-13--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-11.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-14--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, vid=4095, prio=7, dei=1, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-14--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-14.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-15--nok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, vid=4095, prio=7, dei=2, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(eth-15--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME eth-16--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, vid=4095, prio=7, dei=1, vtype=1, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-16--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-14.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-17--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, vid=4095, prio=7, dei=1, vtype=2, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-17--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-17.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-18--nok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, vid=4095, prio=7, dei=1, vtype=0, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(eth-18--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME eth-19--nok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, vid=4095, prio=7, dei=1, vtype=3, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(eth-19--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME eth-20--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, vid=4095, prio=3, ethertype=0x9000, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-20--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-20.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-21--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, vid=1, vid=4095, prio=3, ethertype=0x9000, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-21--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-21.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-22--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, vid=1, vtype=2, vid=4095, prio=3, ethertype=0x9000, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-22--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-22.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-23--nok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, dsap=0, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(eth-23--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME eth-24--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, dsap=0, ssap=0xff, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-24--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-24.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-25--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, dsap=255, ssap=0, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-25--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-25.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-26--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, dsap=255, ssap=0, control=3, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-26--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-25.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-27--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, dsap=255, ssap=0, control=0, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-27--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-27.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-28--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, dsap=255, ssap=0, control=0xffff, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-28--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-28.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-29--nok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, dsap=255, ssap=0, control=0x10000, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(eth-29--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME eth-30--nok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, dsap=256, ssap=0, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(eth-30--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME eth-31--nok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, dsap=0, ssap=256, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(eth-31--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME eth-32--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, ethertype=0x9000, dsap=255, ssap=0, control=0xffff, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-32--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-28.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-33--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, vid=42, dsap=255, ssap=0, control=0xffff, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-33--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-33.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-34--nok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, oui=0, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(eth-34--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME eth-35--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, oui=0, protocol=0, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-35--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-35.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-36--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, oui=0xffffff, protocol=0, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-36--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-36.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-37--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, oui=0x654321, protocol=0xffff, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-37--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-37.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-38--nok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, oui=0x654321, protocol=0x10000, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(eth-38--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME eth-39--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, vid=1, vid=2, oui=0x654321, protocol=0xffff, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-39--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-39.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(NAME arp-1--ok COMMAND tcppump ${DEFAULT_PARAMS} "arp(dip=10.20.30.40)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME arp-1--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/arp-01.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME arp-2--nok COMMAND tcppump ${DEFAULT_PARAMS} "arp(dip=10.20.30.40.50)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(arp-2--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME arp-3--ok COMMAND tcppump ${DEFAULT_PARAMS} "arp(dip=10.20.30.40, op=1)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME arp-3--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/arp-01.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME arp-4--ok COMMAND tcppump ${DEFAULT_PARAMS} "arp(dip=10.20.30.40, op=2)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME arp-4--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/arp-04.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME arp-5--ok COMMAND tcppump ${DEFAULT_PARAMS} "arp(dip=10.20.30.40, op=0xffff)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME arp-5--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/arp-05.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME arp-6--nok COMMAND tcppump ${DEFAULT_PARAMS} "arp(dip=10.20.30.40, op=0x10000)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(arp-6--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME arp-7--ok COMMAND tcppump ${DEFAULT_PARAMS} "arp(dip=10.20.30.40, smac=80:81:82:83:84:85)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME arp-7--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/arp-07.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME arp-8--ok COMMAND tcppump ${DEFAULT_PARAMS} "arp(dip=10.20.30.40, sip=11.22.33.44)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME arp-8--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/arp-08.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME arp-9--ok COMMAND tcppump ${DEFAULT_PARAMS} "arp(dip=10.20.30.40, dmac=80:81:82:83:84:85)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME arp-9--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/arp-09.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME arp-10--ok COMMAND tcppump ${DEFAULT_PARAMS} "arp(op=2, dip=10.20.30.40, dmac=80:81:82:83:84:85, smac=90:91:92:93:94:95, sip=11.22.33.44)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME arp-10--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/arp-10.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME arp-11--ok COMMAND tcppump ${DEFAULT_PARAMS} "arp(dip=10.20.30.40, vid=42, prio=5, dei=1)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME arp-11--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/arp-11.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME arp-12--ok COMMAND tcppump ${DEFAULT_PARAMS} "arp-probe(dip=10.20.30.40)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME arp-12--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/arp-12.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME arp-13--ok COMMAND tcppump ${DEFAULT_PARAMS} "arp-announce(dip=10.20.30.40)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME arp-13--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/arp-13.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(NAME ipv4-1--nok COMMAND tcppump ${DEFAULT_PARAMS} "ipv4(dip=10.20.30.40)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(ipv4-1--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME ipv4-2--nok COMMAND tcppump ${DEFAULT_PARAMS} "ipv4(dip=10.20.30.40, dmac=11:22:33:44:55:66)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(ipv4-2--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME ipv4-3--nok COMMAND tcppump ${DEFAULT_PARAMS} "ipv4(dip=10.20.30.40, dmac=11:22:33:44:55:66, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(ipv4-3--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME ipv4-4--ok COMMAND tcppump ${DEFAULT_PARAMS} "ipv4(dip=10.20.30.40, dmac=11:22:33:44:55:66, protocol=255, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-4--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/ipv4-04.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-5--ok COMMAND tcppump ${DEFAULT_PARAMS} "ipv4(dip=10.20.30.40, dmac=11:22:33:44:55:66, protocol=0, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-5--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/ipv4-05.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-6--nok COMMAND tcppump ${DEFAULT_PARAMS} "ipv4(dip=10.20.30.40, dmac=11:22:33:44:55:66, protocol=256, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(ipv4-6--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME ipv4-6--ok COMMAND tcppump ${DEFAULT_PARAMS} "ipv4(dip=10.20.30.40, dmac=11:22:33:44:55:66, dscp=0, protocol=255, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-6--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/ipv4-04.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-7--ok COMMAND tcppump ${DEFAULT_PARAMS} "ipv4(dip=10.20.30.40, dmac=11:22:33:44:55:66, dscp=0x3f, protocol=255, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-7--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/ipv4-07.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-8--nok COMMAND tcppump ${DEFAULT_PARAMS} "ipv4(dip=10.20.30.40, dmac=11:22:33:44:55:66, dscp=0x40, protocol=255, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(ipv4-8--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME ipv4-9--ok COMMAND tcppump ${DEFAULT_PARAMS} "ipv4(dip=10.20.30.40, dmac=11:22:33:44:55:66, dscp=0x3f, ecn=0, protocol=255, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-9--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/ipv4-07.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-10--ok COMMAND tcppump ${DEFAULT_PARAMS} "ipv4(dip=10.20.30.40, dmac=11:22:33:44:55:66, dscp=0x0, ecn=3, protocol=255, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-10--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/ipv4-10.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-11--nok COMMAND tcppump ${DEFAULT_PARAMS} "ipv4(dip=10.20.30.40, dmac=11:22:33:44:55:66, dscp=0x0, ecn=4, protocol=255, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(ipv4-11--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME ipv4-12--ok COMMAND tcppump ${DEFAULT_PARAMS} "ipv4(dip=10.20.30.40, dmac=11:22:33:44:55:66, ttl=64, dscp=0x0, ecn=3, protocol=255, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-12--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/ipv4-10.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-13--ok COMMAND tcppump ${DEFAULT_PARAMS} "ipv4(dip=10.20.30.40, dmac=11:22:33:44:55:66, ttl=0, dscp=0x0, ecn=3, protocol=255, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-13--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/ipv4-13.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-14--ok COMMAND tcppump ${DEFAULT_PARAMS} "ipv4(dip=10.20.30.40, dmac=11:22:33:44:55:66, ttl=255, dscp=0x0, ecn=3, protocol=255, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-14--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/ipv4-14.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-15--nok COMMAND tcppump ${DEFAULT_PARAMS} "ipv4(dip=10.20.30.40, dmac=11:22:33:44:55:66, ttl=256, dscp=0x0, ecn=3, protocol=255, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(ipv4-15--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME ipv4-16--ok COMMAND tcppump ${DEFAULT_PARAMS} "ipv4(dip=10.20.30.40, dmac=11:22:33:44:55:66, df=0, ttl=255, dscp=0x0, ecn=3, protocol=255, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-16--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/ipv4-14.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-17--ok COMMAND tcppump ${DEFAULT_PARAMS} "ipv4(dip=10.20.30.40, dmac=11:22:33:44:55:66, df=1, ttl=255, dscp=0x0, ecn=3, protocol=255, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-17--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/ipv4-17.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-18--nok COMMAND tcppump ${DEFAULT_PARAMS} "ipv4(dip=10.20.30.40, dmac=11:22:33:44:55:66, df=2, ttl=255, dscp=0x0, ecn=3, protocol=255, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(ipv4-18--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME ipv4-19--ok COMMAND tcppump ${DEFAULT_PARAMS} "ipv4(dip=10.20.30.40, smac=90:91:92:93:94:95, dmac=11:22:33:44:55:66, df=1, ttl=255, dscp=0x0, ecn=3, protocol=255, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-19--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/ipv4-19.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-20--ok COMMAND tcppump ${DEFAULT_PARAMS} "ipv4(sip=11.22.33.44, dip=10.20.30.40, smac=90:91:92:93:94:95, dmac=11:22:33:44:55:66, df=1, ttl=255, dscp=0x0, ecn=3, protocol=255, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-20--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/ipv4-20.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-21--ok COMMAND tcppump ${DEFAULT_PARAMS} "ipv4(vid=42, prio=4, sip=11.22.33.44, dip=10.20.30.40, smac=90:91:92:93:94:95, dmac=11:22:33:44:55:66, df=1, ttl=255, dscp=0x0, ecn=3, protocol=255, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME ipv4-21--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/ipv4-21.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(NAME udp-1--nok COMMAND tcppump ${DEFAULT_PARAMS} "udp()" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(udp-1--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME udp-2--nok COMMAND tcppump ${DEFAULT_PARAMS} "udp(dmac=11:22:33:44:55:66)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(udp-2--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME udp-3--nok COMMAND tcppump ${DEFAULT_PARAMS} "udp(dip=10.20.30.40, dmac=11:22:33:44:55:66)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(udp-3--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME udp-4--nok COMMAND tcppump ${DEFAULT_PARAMS} "udp(dip=10.20.30.40, dmac=11:22:33:44:55:66, sport=0)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(udp-4--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME udp-5--ok COMMAND tcppump ${DEFAULT_PARAMS} "udp(dip=10.20.30.40, dmac=11:22:33:44:55:66, sport=0, dport=0)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME udp-5--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/udp-05.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME udp-6--ok COMMAND tcppump ${DEFAULT_PARAMS} "udp(dip=10.20.30.40, dmac=11:22:33:44:55:66, sport=0xffff, dport=0)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME udp-6--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/udp-06.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME udp-7--nok COMMAND tcppump ${DEFAULT_PARAMS} "udp(dip=10.20.30.40, dmac=11:22:33:44:55:66, sport=0x10000, dport=0)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(udp-7--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME udp-8--ok COMMAND tcppump ${DEFAULT_PARAMS} "udp(dip=10.20.30.40, dmac=11:22:33:44:55:66, sport=0, dport=0xffff)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME udp-8--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/udp-08.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME udp-9--nok COMMAND tcppump ${DEFAULT_PARAMS} "udp(dip=10.20.30.40, dmac=11:22:33:44:55:66, sport=0, dport=0x10000)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(udp-9--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME udp-10--ok COMMAND tcppump ${DEFAULT_PARAMS} "udp(dip=10.20.30.40, dmac=11:22:33:44:55:66, sport=1, dport=2, chksum=0)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME udp-10--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/udp-10.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME udp-11--ok COMMAND tcppump ${DEFAULT_PARAMS} "udp(dip=10.20.30.40, dmac=11:22:33:44:55:66, sport=1, dport=2, chksum=0xffff)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME udp-11--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/udp-11.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME udp-12--nok COMMAND tcppump ${DEFAULT_PARAMS} "udp(dip=10.20.30.40, dmac=11:22:33:44:55:66, sport=1, dport=2, chksum=0x10000)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(udp-12--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME udp-13--ok COMMAND tcppump ${DEFAULT_PARAMS} "udp(dip=10.20.30.40, dmac=11:22:33:44:55:66, sport=1, dport=2, chksum=0xffff,  payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME udp-13--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/udp-13.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME udp-14--ok COMMAND tcppump ${DEFAULT_PARAMS} "udp(dip=10.20.30.40, dmac=11:22:33:44:55:66, sport=1, dport=2, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME udp-14--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/udp-14.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME udp-15--ok COMMAND tcppump ${DEFAULT_PARAMS} "udp(sip=11.22.33.44, smac=90:91:92:93:94:95, dip=10.20.30.40, dmac=11:22:33:44:55:66, sport=1, dport=2, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME udp-15--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/udp-15.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME udp-16--ok COMMAND tcppump ${DEFAULT_PARAMS} "udp(dscp=42, ecn=2, ttl=10, df=1, sip=11.22.33.44, smac=90:91:92:93:94:95, dip=10.20.30.40, dmac=11:22:33:44:55:66, sport=1, dport=2, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME udp-16--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/udp-16.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME udp-17--ok COMMAND tcppump ${DEFAULT_PARAMS} "udp(vid=42, vtype=2, dei=1, prio=4, vid=43, prio=5, dscp=42, ecn=2, ttl=10, df=1, sip=11.22.33.44, smac=90:91:92:93:94:95, dip=10.20.30.40, dmac=11:22:33:44:55:66, sport=1, dport=2, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME udp-17--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/udp-17.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})


add_test(NAME script-01--ok COMMAND tcppump ${DEFAULT_PARAMS} -s ${TEST_FILE_PREFIX}/all.pump WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME script-01--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/all.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})


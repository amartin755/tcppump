# SPDX-License-Identifier: GPL-3.0-only
###############################################################################
#
# TCPPUMP <https://github.com/amartin755/tcppump>
# Copyright (C) 2012-2024 Andreas Martin (netnag@mailbox.org)
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
cmake_minimum_required (VERSION 3.19)

project (tcppump VERSION 0.4.0
                 DESCRIPTION "Ethernet packet generator"
                 LANGUAGES CXX)
###############################################################################

option(WITH_UNITTESTS "enable/disable generation of unittest binary (default ON)" ON)
option(WITH_ASAN      "enable/disable address sanitizer (default OFF)" OFF)
option(WITH_UBSAN     "enable/disable undefined behavior sanitizer (default OFF)" OFF)
option(WITH_GCOV      "enable/disable code coverage profiling (default OFF)" OFF)
option(WITH_WERROR    "enable/disable compilation warnings as errors (default OFF)" OFF)
option(WITH_CPPCHECK  "enable/disable cppcheck (default OFF)" OFF)

# set output directories to "bin"
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/../bin)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/../bin)

# cmake includes
###############################################################################
list (APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/src/cmake)
include (CheckLibraryExists)
include (CheckSymbolExists)
include (CheckFunctionExists)
include (CheckIncludeFiles)
include (TestBigEndian)
include (CTest)
include (gitinfo)
include (download)
include (utils)

# compiler settings
###############################################################################
# C++11 is required
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# warning level
if (MSVC)
    add_compile_options (/W3 /wd5208)
    if (WITH_WERROR)
        add_compile_options (/WX)
    endif ()
else()
    add_compile_options (-Wall -Wextra -Wpedantic)
    if (WITH_WERROR)
        add_compile_options (-Werror)
    endif ()
endif()

# on mingw we link the std library statically
if (MINGW)
    add_link_options (-static-libgcc -static-libstdc++)
endif ()

if (WITH_UBSAN)
    add_compile_options (-fsanitize=undefined -fno-sanitize-recover=all)
    add_link_options (-fsanitize=undefined -fno-sanitize-recover=all)
endif ()

if (WITH_ASAN)
    add_compile_options (-fsanitize=address -fno-sanitize-recover=all)
    add_link_options (-fsanitize=address -fno-sanitize-recover=all)
endif ()

if (WITH_GCOV)
    add_compile_options (--coverage)
    add_link_options (--coverage)
endif ()

if (WITH_CPPCHECK)
    find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)
    if (CMAKE_CXX_CPPCHECK)
        list (APPEND CMAKE_CXX_CPPCHECK "--check-level=exhaustive")
        list (APPEND CMAKE_CXX_CPPCHECK "--enable=warning")
        list (APPEND CMAKE_CXX_CPPCHECK "-q")
        list (APPEND CMAKE_CXX_CPPCHECK "--suppress=duplInheritedMember")
        list (APPEND CMAKE_CXX_CPPCHECK "--inline-suppr")
    endif ()
endif ()

# download dependencies
###############################################################################
if (WIN32)
    if(NOT EXISTS ${PROJECT_SOURCE_DIR}/libpcap)
        download_file(https://www.winpcap.org/install/bin/WpdPack_4_1_2.zip ./winpcap.zip
           SHA1 f5c80885bd48f07f41833d0f65bf85da1ef1727a)
        execute_process (COMMAND ${CMAKE_COMMAND} -E tar xzf ./winpcap.zip WORKING_DIRECTORY .)
        execute_process (COMMAND ${CMAKE_COMMAND} -E copy_directory ./WpdPack/Lib ${PROJECT_SOURCE_DIR}/libpcap/lib)
        execute_process (COMMAND ${CMAKE_COMMAND} -E copy_directory ./WpdPack/Include ${PROJECT_SOURCE_DIR}/libpcap/include)
        execute_process (COMMAND ${CMAKE_COMMAND} -E remove_directory ./WpdPack)
        execute_process (COMMAND ${CMAKE_COMMAND} -E remove ./winpcap.zip)
    endif ()
endif ()

# check dependencies
###############################################################################

# try to find pcap library; can be set via PCAP_INCLUDE_DIR and PCAP_LIBRARY
if (NOT PCAP_INCLUDE_DIR)
    set (PCAPDIR ${PROJECT_SOURCE_DIR}/libpcap/include)
    find_path (PCAP_INCLUDE_DIR NAMES pcap/pcap.h pcap.h PATHS ${PCAPDIR})
endif ()
if (NOT PCAP_LIBRARY)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        # 64 bits
        set (PCAPDIR ${PROJECT_SOURCE_DIR}/libpcap/lib/x64)
    else ()
        # 32 bits
        set (PCAPDIR ${PROJECT_SOURCE_DIR}/libpcap/lib)
    endif()
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

if (NOT HAVE_PCAP)
    message (FATAL_ERROR "Library pcap not found!")
endif ()


if (WIN32)
    # winsock, winpcap, conio and iphlpapi are needed on windows
    check_include_files("winsock2.h;iphlpapi.h;winerror.h" HAVE_WS2)
    # check for inet_pton() inet_ntop(), they do not exist on older mingw
    set (CMAKE_REQUIRED_LIBRARIES "ws2_32")
    set (CMAKE_REQUIRED_DEFINITIONS -D_WIN32_WINNT=0x0600)
    check_symbol_exists (inet_pton "ws2tcpip.h" HAVE_PTON)
    check_symbol_exists (inet_ntop "ws2tcpip.h" HAVE_NTOP)
    set (CMAKE_REQUIRED_LIBRARIES "")
    set (CMAKE_REQUIRED_DEFINITIONS "")

    if (NOT HAVE_WS2)
        message (FATAL_ERROR "Winsock and ip helper headers not found!")
    endif ()
else ()
    check_symbol_exists (inet_pton "arpa/inet.h" HAVE_PTON)
    check_symbol_exists (inet_ntop "arpa/inet.h" HAVE_NTOP)
endif ()

check_include_files("sys/time.h" HAVE_SYSTIME_H)
check_include_files("arpa/inet.h" HAVE_ARPAINET_H)
check_include_files("byteswap.h" HAVE_BYTESWAP_H)
set (CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE=1)
check_symbol_exists (memmem "string.h" HAVE_MEMMEM)
set (CMAKE_REQUIRED_DEFINITIONS "")
# check endianess
test_big_endian (HAVE_BIG_ENDIAN)


# preprocessor definitions
###############################################################################
add_compile_definitions (APP_VERSION="${PROJECT_VERSION}")
add_compile_definitions ($<$<CONFIG:Release>:_FORTIFY_SOURCE=2>)

if (HAVE_WS2)
    add_compile_definitions (HAVE_WS2)
endif ()
if (HAVE_SYSTIME_H)
    add_compile_definitions (HAVE_SYSTIME_H)
endif ()
if (HAVE_ARPAINET_H)
    add_compile_definitions (HAVE_ARPAINET_H)
endif ()
if (HAVE_BYTESWAP_H)
    add_compile_definitions (HAVE_BYTESWAP_H)
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
if (HAVE_MEMMEM)
    add_compile_definitions (HAVE_MEMMEM)
endif ()
if (HAVE_BIG_ENDIAN)
    add_compile_definitions (HAVE_BIG_ENDIAN)
endif ()
#add_compile_definitions (_FORTIFY_SOURCE=2)
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
endif ()

# generate build numbers
###############################################################################
git_info ()
add_compile_definitions (GIT_COMMIT_SHORT="${GIT_COMMIT_SHORT}")
add_compile_definitions (GIT_TAG="${GIT_TAG}")
add_compile_definitions (GIT_BRANCH="${GIT_BRANCH}")
add_compile_definitions (GIT_COMMIT="${GIT_COMMIT}")
string(TIMESTAMP BUILD_TIME "%Y-%m-%d %H:%M" UTC)
add_compile_definitions(BUILD_TIME="${BUILD_TIME}")
add_compile_definitions (BUILD_TYPE="${CMAKE_BUILD_TYPE}")

message (STATUS "Build infos:")
message (STATUS "  unittests  = ${WITH_UNITTESTS}")
message (STATUS "  ASAN       = ${WITH_ASAN}")
message (STATUS "  UBSAN      = ${WITH_UBSAN}")
message (STATUS "  GCOV       = ${WITH_GCOV}")
message (STATUS "  type       = ${CMAKE_BUILD_TYPE}")
message (STATUS "  timestamp  = ${BUILD_TIME}")
message (STATUS "  git tag    = ${GIT_TAG}")
message (STATUS "  git branch = ${GIT_BRANCH}")
message (STATUS "  git commit = ${GIT_COMMIT}")

# source files
###############################################################################

# platform specific libs
if (WIN32)
    set (LIBS
         ws2_32
         IPHLPAPI)
endif ()

# includes and libs
if (HAVE_PCAP)
    set (LIBS ${LIBS} ${__PCAP_LIBRARY})
    if (PCAP_INCLUDE_DIR)
        include_directories (${PCAP_INCLUDE_DIR})
    endif ()
endif ()

add_subdirectory (libcmdline)

# target tcppump (main binary)
###############################################################################
add_executable (tcppump)
add_subdirectory (src)
target_link_libraries (tcppump PRIVATE ${LIBS} cmdline)
target_sources (tcppump PRIVATE ${MAIN_SOURCES})
target_include_directories (tcppump PRIVATE ${INCLUDES})


# target setcap (helper target to set raw capabilities)
###############################################################################
if (UNIX)
    # add raw capablities in oder to be runable without root
    add_custom_target (setcap
                       DEPENDS tcppump
                       COMMAND sudo setcap cap_net_raw+eip ./tcppump
                       WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()


# target unittest (unit test code)
###############################################################################
if (WITH_UNITTESTS)
    add_executable (unittest)

    target_link_libraries (unittest PRIVATE ${LIBS} cmdline)
    target_compile_definitions (unittest PRIVATE WITH_UNITTESTS)
    target_sources (unittest PRIVATE ${UTEST_SOURCES})
    target_include_directories (unittest PRIVATE ${INCLUDES})
endif ()


# install & package
###############################################################################
install(TARGETS tcppump)

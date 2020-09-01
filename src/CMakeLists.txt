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

project (tcppump VERSION 0.0.1.0
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
find_library (PCAP_LIBRARY NAMES wpcap pcap PATHS ${PCAPDIR})
message (STATUS "PCAP_INCLUDE_DIR = ${PCAP_INCLUDE_DIR}")
message (STATUS "PCAP_LIBRARY     = ${PCAP_LIBRARY}")
set (CMAKE_REQUIRED_LIBRARIES ${PCAP_LIBRARY})
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
    set (libs ${libs} ${PCAP_LIBRARY})
    if (PCAP_INCLUDE_DIR)
        include_directories (${PCAP_INCLUDE_DIR})
    endif ()
endif ()

# note: include only source files here, that are used by all targets
set (sourcefiles
     dissector.cpp
     libnetnag/cmdline.cpp
     libnetnag/console.cpp
     libnetnag/converter.cpp
     libnetnag/ethernetpacket.cpp
     libnetnag/arppacket.cpp
     libnetnag/ipv4packet.cpp
     libnetnag/instructionparser.cpp
     libnetnag/fileparser.cpp
     libnetnag/parameterlist.cpp
     libnetnag/parsehelper.cpp
     libnetnag/pcapfileio.cpp
     libnetnag/udppacket.cpp
     libnetnag/vrrppacket.cpp
     ${os_specific}/interface.cpp
     ${os_specific}/getch.cpp
     ${os_specific}/sleep.cpp
     )


add_executable (tcppump tcppump.cpp ${sourcefiles})

target_link_libraries (tcppump LINK_PUBLIC ${libs})
target_include_directories (tcppump PUBLIC ${os_specific})
target_include_directories (tcppump PUBLIC os)
target_include_directories (tcppump PUBLIC libnetnag)

# unit test code
###############################################################################
if (WITH_UNITTESTS)
    add_executable (unittest test/unittest.cpp ${sourcefiles})
    target_link_libraries (unittest PRIVATE ${libs})

    target_compile_definitions (unittest PRIVATE WITH_UNITTESTS)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options (unittest PRIVATE -Wno-write-strings --coverage)
        target_link_libraries (unittest PRIVATE gcov)
    endif()

    target_include_directories (unittest PUBLIC ${os_specific})
    target_include_directories (unittest PUBLIC os)
    target_include_directories (unittest PUBLIC libnetnag)
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
set (TEST_OUTPUT testoutput.pcap)
set (DEFAULT_PARAMS -i ${OUT_IFC} --myip4=1.2.3.4 --mymac=81:23:45:67:89:AB -o ${TEST_OUTPUT})
set (TEST_FILE_PREFIX ${CMAKE_BINARY_DIR}/../src/test)


add_test(NAME no-params--nok COMMAND tcppump WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(no-params--nok PROPERTIES WILL_FAIL TRUE)

add_test(NAME show-usage--ok COMMAND tcppump -h WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(NAME raw-1--ok COMMAND tcppump ${DEFAULT_PARAMS} -r "0123456789abcdef0123456789abcdef" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME raw-1--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/raw-1.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME raw-2--nok COMMAND tcppump ${DEFAULT_PARAMS} -r "abcdf" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(raw-2--nok PROPERTIES WILL_FAIL TRUE)

add_test(NAME raw-3--ok COMMAND tcppump ${DEFAULT_PARAMS} "raw(payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME raw-3--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/raw-3.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(NAME eth-1--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-1--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-1.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-2--nok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=1122:33:44:55:66, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(eth-2--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME eth-3--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, ethertype=0x9000, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-3--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-3.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-4--nok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, ethertype=0x10000, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(eth-4--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME eth-5--nok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set_tests_properties(eth-5--nok PROPERTIES WILL_FAIL TRUE)
add_test(NAME eth-6--ok COMMAND tcppump ${DEFAULT_PARAMS} "eth(dmac=11:22:33:44:55:66, smac=80:81:82:83:84:85, payload = 0123456789abcdef0123456789abcdef)" WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(NAME eth-6--diff COMMAND cmake -E compare_files ${TEST_OUTPUT} ${TEST_FILE_PREFIX}/eth-6.pcap WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

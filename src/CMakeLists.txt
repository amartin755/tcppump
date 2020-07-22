###############################################################################
#
# TCPPUMP <https://github.com/amartin755/tcppump>
# Copyright (C) 2019 Andreas Martin (netnag@mailbox.org)
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
CMAKE_MINIMUM_REQUIRED (VERSION 3.0.0)

PROJECT (tcppump VERSION 0.0.1.0)
###############################################################################


# set output directories to "bin"
SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

# check environment
###############################################################################
INCLUDE (CheckLibraryExists)
INCLUDE (CheckSymbolExists)
INCLUDE (CheckFunctionExists)
INCLUDE (CheckIncludeFiles)

# try to find pcap library; can be set via PCAP_INCLUDE_DIR and PCAP_LIBRARY
IF (NOT PCAP_INCLUDE_DIR)
    SET (PCAPDIR ${PROJECT_SOURCE_DIR}/../pcap/include)
    FIND_PATH (PCAP_INCLUDE_DIR NAMES pcap/pcap.h pcap.h PATHS ${PCAPDIR} NO_DEFAULT_PATH)
ENDIF (NOT PCAP_INCLUDE_DIR)
IF (NOT PCAP_LIBRARY)
    SET (PCAPDIR ${PROJECT_SOURCE_DIR}/../pcap/lib)
    FIND_LIBRARY (PCAP_LIBRARY NAMES wpcap pcap PATHS ${PCAPDIR} NO_DEFAULT_PATH)
ENDIF (NOT PCAP_LIBRARY)
IF (NOT PCAP_LIBRARY)
    FIND_LIBRARY (PCAP_LIBRARY NAMES wpcap pcap)
ENDIF (NOT PCAP_LIBRARY)

MESSAGE (STATUS "PCAP_INCLUDE_DIR = ${PCAP_INCLUDE_DIR}")
MESSAGE (STATUS "PCAP_LIBRARY = ${PCAP_LIBRARY}")
SET (CMAKE_REQUIRED_LIBRARIES ${PCAP_LIBRARY})
CHECK_FUNCTION_EXISTS (pcap_open_offline HAVE_PCAP)
SET (CMAKE_REQUIRED_LIBRARIES "")


# on windows winsock, winpcap, conio and iphlpapi is needed
IF (WIN32)
    CHECK_INCLUDE_FILES("winsock2.h;iphlpapi.h;winerror.h" HAVE_WS2)
    # getch() is needed for unbuffered stdin on Windows
    CHECK_SYMBOL_EXISTS (getch conio.h HAVE_GETCH)

    IF (NOT HAVE_GETCH)
        MESSAGE (FATAL_ERROR "Function getch() not found!")
    ENDIF (NOT HAVE_GETCH)
    IF (NOT HAVE_WS2)
        MESSAGE (FATAL_ERROR "Winsock and ip helper headers not found!")
    ENDIF (NOT HAVE_WS2)
    IF (NOT HAVE_PCAP)
        MESSAGE (FATAL_ERROR "Library wpcap not found!")
    ENDIF (NOT HAVE_PCAP)
ENDIF (WIN32)

CHECK_INCLUDE_FILES("sys/time.h" HAVE_SYSTIME_H)
CHECK_INCLUDE_FILES("arpa/inet.h" HAVE_ARPAINET_H)


# definitions
###############################################################################
ADD_DEFINITIONS (-DAPP_VERSION="${PROJECT_VERSION}")
IF (HAVE_WS2)
    ADD_DEFINITIONS (-DHAVE_WS2)
ENDIF (HAVE_WS2)
IF (HAVE_SYSTIME_H)
    ADD_DEFINITIONS (-DHAVE_SYSTIME_H)
ENDIF (HAVE_SYSTIME_H)
IF (HAVE_ARPAINET_H)
    ADD_DEFINITIONS (-DHAVE_ARPAINET_H)
ENDIF (HAVE_ARPAINET_H)
IF (HAVE_PCAP)
    ADD_DEFINITIONS (-DHAVE_PCAP)
ENDIF (HAVE_PCAP)
IF (WIN32)
    # min. supported version is Windows 7
    ADD_DEFINITIONS (-D_WIN32_WINNT=0x0601)
    ADD_DEFINITIONS (-DWINVER=0x0601)
ENDIF (WIN32)
add_definitions(-DWITH_TIMESTAMP)


# warning level
###############################################################################
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# on mingw we link the std library statically
IF (MINGW)
    SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
ENDIF (MINGW)


# source files
###############################################################################

# platform specific libs and files
IF (WIN32)
    SET (os_specific os/win32)
    SET (libs
         ws2_32
         IPHLPAPI)
ENDIF (WIN32)
IF (UNIX)
    SET (os_specific os/posix)
ENDIF (UNIX)

# includes and libs
IF (HAVE_PCAP)
    SET (libs ${libs} ${PCAP_LIBRARY})
    IF (PCAP_INCLUDE_DIR)
        INCLUDE_DIRECTORIES (${PCAP_INCLUDE_DIR})
    ENDIF (PCAP_INCLUDE_DIR)
ENDIF (HAVE_PCAP)

# note: include only source files here, that are used by all targets
SET (sourcefiles
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
     libnetnag/pcapfileio.cpp
     libnetnag/udppacket.cpp
     ${os_specific}/interface.cpp
     ${os_specific}/sleep.cpp
     ${os_specific}/getch.cpp
     )


ADD_EXECUTABLE (tcppump tcppump.cpp ${sourcefiles})
TARGET_LINK_LIBRARIES (tcppump LINK_PUBLIC ${libs})

TARGET_INCLUDE_DIRECTORIES (tcppump PUBLIC ${os_specific})
TARGET_INCLUDE_DIRECTORIES (tcppump PUBLIC os)
TARGET_INCLUDE_DIRECTORIES (tcppump PUBLIC libnetnag)

# unit test code
###############################################################################
IF (WITH_UNITTESTS)
	ADD_EXECUTABLE (unittest test/unittest.cpp ${sourcefiles})
	TARGET_LINK_LIBRARIES (unittest LINK_PUBLIC ${libs})
	TARGET_COMPILE_DEFINITIONS (unittest PRIVATE -DWITH_UNITTESTS)
	TARGET_COMPILE_OPTIONS (unittest PRIVATE -Wno-write-strings)

	TARGET_INCLUDE_DIRECTORIES (unittest PUBLIC ${os_specific})
	TARGET_INCLUDE_DIRECTORIES (unittest PUBLIC os)
	TARGET_INCLUDE_DIRECTORIES (unittest PUBLIC libnetnag)
ENDIF (WITH_UNITTESTS)

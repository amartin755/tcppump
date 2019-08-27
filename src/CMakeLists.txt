###############################################################################
CMAKE_MINIMUM_REQUIRED (VERSION 3.0.0)

PROJECT (tcppump VERSION 0.0.1.0)

###############################################################################


INCLUDE (CheckSymbolExists)


# set output directories to "bin"
SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

# check environment
###############################################################################

# commandline parsing is based on GNUs getopt_long()
CHECK_SYMBOL_EXISTS (getopt_long getopt.h HAVE_GETOPT_LONG)
IF (NOT HAVE_GETOPT_LONG)
MESSAGE (FATAL_ERROR "Symbol getopt_long not found!")
ENDIF (NOT HAVE_GETOPT_LONG)

# getch() is needed for unbuffered stdin on Windows
CHECK_SYMBOL_EXISTS (getch conio.h HAVE_GETCH)


# definitions
###############################################################################
ADD_DEFINITIONS(-DAPP_VERSION="${PROJECT_VERSION}")
IF (WIN32)
    # min. supported version is Windows 7
    ADD_DEFINITIONS(-D_WIN32_WINNT=0x0601)
    ADD_DEFINITIONS(-DWINVER=0x0601)
ENDIF (WIN32)


# warning level
###############################################################################
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# on mingw we link the std library statically
IF (MINGW)
    SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
ENDIF (MINGW)


# source files
###############################################################################

# platform specific libs and files
IF (WIN32)
	# on windows we need winpcap
	FIND_PATH (WINPCAP_INCLUDE_DIR pcap.h "${PROJECT_SOURCE_DIR}/../pcap/*")
	INCLUDE_DIRECTORIES (${WINPCAP_INCLUDE_DIR})
	FIND_LIBRARY (WINPCAP_LIBRARY NAMES "wpcap" PATHS "${PROJECT_SOURCE_DIR}/../pcap/*")
	
	SET (os_specific os/win32)
	SET (libs 
	     ws2_32 
	     IPHLPAPI 
	     ${WINPCAP_LIBRARY})
ENDIF (WIN32)
IF (UNIX)
	SET (os_specific os/posix)
ENDIF (UNIX)


SET (sourcefiles
     tcppump.cpp
     dissector.cpp
     libnetnag/cmdline.cpp
     libnetnag/console.cpp
     libnetnag/converter.cpp
     libnetnag/ethernetpacket.cpp
     libnetnag/instructionparser.cpp
     libnetnag/fileparser.cpp
     libnetnag/parameterlist.cpp
     ${os_specific}/interface.cpp
     ${os_specific}/sleep.cpp
     ${os_specific}/getch.cpp
     )


ADD_EXECUTABLE (tcppump ${sourcefiles})
TARGET_LINK_LIBRARIES (tcppump LINK_PUBLIC ${libs})

TARGET_INCLUDE_DIRECTORIES (tcppump PUBLIC ${os_specific})
TARGET_INCLUDE_DIRECTORIES (tcppump PUBLIC os)
TARGET_INCLUDE_DIRECTORIES (tcppump PUBLIC libnetnag)


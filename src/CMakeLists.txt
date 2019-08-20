###############################################################################
CMAKE_MINIMUM_REQUIRED (VERSION 3.0.0)

PROJECT (tcppump VERSION 0.0.1.0)

###############################################################################


INCLUDE (CheckIncludeFiles)


# set output directories to "bin"
SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

# check environment
###############################################################################

# commandline parsing is based on GNUs getopt_long()
CHECK_INCLUDE_FILES (getopt.h HAVE_GETOPT_H)
IF (NOT HAVE_GETOPT_H)
MESSAGE (FATAL_ERROR "getopt.h not found!")
ENDIF (NOT HAVE_GETOPT_H)

# definitions
###############################################################################
add_definitions(-DAPP_VERSION="${PROJECT_VERSION}")


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
SET (sourcefiles
     interface.cpp
     tcppump.cpp
     libnetnag/cmdline.cpp
     libnetnag/console.cpp
     libnetnag/converter.cpp
     libnetnag/ethernetpacket.cpp
     libnetnag/instructionparser.cpp
     libnetnag/fileparser.cpp
     libnetnag/parameterlist.cpp)

# platform specific files
IF (WIN32)
	SET (sourcefiles ${sourcefiles}
	     libnetnag/system_win32.cpp)
	SET (libs ws2_32)
ENDIF (WIN32)
IF (UNIX)
	SET (sourcefiles ${sourcefiles}
	     libnetnag/system_posix.cpp)
ENDIF (UNIX)


ADD_EXECUTABLE (tcppump ${sourcefiles})
TARGET_LINK_LIBRARIES (tcppump LINK_PUBLIC ${libs})

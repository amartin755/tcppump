
# list of source files
SET (sourcefiles
     interface.cpp
     tcppump.cpp
     libnetnag/cmdline.cpp
     libnetnag/console.cpp
     libnetnag/converter.cpp)

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

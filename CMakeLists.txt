cmake_minimum_required (VERSION 3.0.0)

project (tcppump VERSION 0.0.1.0)

# set output directories to "bin"
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


if (MINGW)
    # set warning level
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-write-strings")

    # on mingw we link the std library statically
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
endif ()



add_subdirectory (src)

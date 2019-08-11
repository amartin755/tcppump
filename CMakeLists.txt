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


if(${WIN32})
    # we must set the minimum supported windows version

    # windows xp
    set(supp_win_ver 0x0501)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=${supp_win_ver}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWINVER=${supp_win_ver}")
    add_definitions(-D_WIN32_WINNT=${supp_win_ver})
    add_definitions(-DWINVER=${supp_win_ver})

    # get rid of deprecated and security warnings
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
endif()

add_executable (tcppump src/cmdline.cpp src/console.cpp src/tcppump.cpp)

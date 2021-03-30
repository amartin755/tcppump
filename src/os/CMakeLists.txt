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

# platform specific files
if (WIN32)
    set (OS_SPECIFIC ${CMAKE_CURRENT_SOURCE_DIR}/win32)
endif ()
if (UNIX)
    set (OS_SPECIFIC ${CMAKE_CURRENT_SOURCE_DIR}/linux)
endif ()

set (SOURCES
     ${SOURCES}
     ${CMAKE_CURRENT_SOURCE_DIR}/netinterface.cpp
     ${OS_SPECIFIC}/interface.cpp
     ${OS_SPECIFIC}/getch.cpp
     ${OS_SPECIFIC}/sleep.cpp
     ${OS_SPECIFIC}/signal.cpp
     ${OS_SPECIFIC}/arp.cpp
     PARENT_SCOPE
)
set (INCLUDES
     ${INCLUDES}
     ${CMAKE_CURRENT_SOURCE_DIR}
     ${OS_SPECIFIC}
     PARENT_SCOPE
)

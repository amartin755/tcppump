# SPDX-License-Identifier: GPL-3.0-only
###############################################################################
#
# TCPPUMP <https://github.com/amartin755/tcppump>
# Copyright (C) 2012-2020 Andreas Martin (netnag@mailbox.org)
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

set (SOURCES
     ${SOURCES}
     ${CMAKE_CURRENT_SOURCE_DIR}/arppacket.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/ethernetpacket.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/icmppacket.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/igmppacket.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/ipv4packet.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/stppacket.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/tcppacket.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/udppacket.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/vrrppacket.cpp
     PARENT_SCOPE
)
set (INCLUDES
     ${INCLUDES}
     ${CMAKE_CURRENT_SOURCE_DIR}
     PARENT_SCOPE
)

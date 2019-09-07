/**
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2016 Andreas Martin (netnag@mailbox.org)
 *
 * tcppump.hpp
 *
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef TCPPUMP_HPP
#define TCPPUMP_HPP


#include <list>
#include <map>
#include <cstddef>
#include "libnetnag/protocoltypes.hpp"
#include "libnetnag/cmdlineapp.hpp"
#include "libnetnag/ethernetpacket.hpp"

typedef struct
{
	const char*  ifc;
	int          repeat;
	bool         help;
	bool         verbose;
	int          delay;
	bool         interactive;
	bool         raw;
	bool         script;
}appOptions;

class cInterface;


class cTcpPump : public cCmdlineApp
{
public:
	cTcpPump (const char* name, const char* brief, const char* usage, const char* description);
	virtual ~cTcpPump();

	int execute (int argc, char* argv[]);

private:
	bool parsePackets (mac_t ownMac, int cnt, char* packets[]);
	bool parseScripts (mac_t ownMac, int cnt, char* packets[]);
	bool sendPacket (cInterface &ifc, cEthernetPacket& p);

	appOptions options;
	std::list <cEthernetPacket> packets;
	std::map <int, cEthernetPacket&> keyBindings;
};

#endif /* TCPPUMP_HPP */

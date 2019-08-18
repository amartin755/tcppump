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

#include <vector>
#include <cstddef>
#include "libnetnag/cmdlineapp.hpp"

typedef struct
{
	const char*  ifc;
	int          repeat;
	bool         help;
	bool         verbose;
	int          delay;
	bool         interactive;
}appOptions;

typedef struct
{
	int key;
	uint8_t* packet;
	size_t packetLen;
}packet_t;


class cTcpPump : public cCmdlineApp
{
public:
	cTcpPump(const char* name, const char* brief, const char* description);
	virtual ~cTcpPump();

	int execute (int argc, char* argv[]);

private:
	appOptions options;
	std::vector <packet_t> packets;
};

#endif /* TCPPUMP_HPP */

/**
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2019 Andreas Martin (netnag@mailbox.org)
 *
 * tcppump.cpp
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

#include "tcppump.hpp"



cTcpPump::cTcpPump(const char* name, const char* brief, const char* description)
: cCmdlineApp (name, brief, description)
{
	memset (&options, 0, sizeof(options));
	options.repeat = 1;

	cmdline.addOption ('i', "interface", "IFC", "PCAP interface name", &options.ifc, true);
	cmdline.addOption ('r', "reoeat", "CNT", "Send the file/frame CNT times", &options.repeat, true);
	cmdline.addOption ('v', "verbose", "Enable verbose mode", &options.verbose, true);
}

cTcpPump::~cTcpPump()
{
	// TODO Auto-generated destructor stub
}

int cTcpPump::execute (int argc, char* argv[])
{
}

int main(int argc, char* argv[])
{
	cTcpPump app ("tcppump -i IFC [OPTIONS] [INPUTFILES]", "A ethernet packet generator", "TODO description");
	return app.main (argc, argv);
}

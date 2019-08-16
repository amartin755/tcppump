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

#include <cstring>
#include <cstdint>

#include "tcppump.hpp"
#include "interface.hpp"
#include "libnetnag/converter.hpp"
#include "libnetnag/system.hpp"

using namespace nn;

cTcpPump::cTcpPump(const char* name, const char* brief, const char* description)
: cCmdlineApp (name, brief, description)
{
	memset (&options, 0, sizeof(options));
	options.repeat = 1;
	options.delay  = 0;

	cmdline.addOption ('i', "interface", "IFC", "Interface name", &options.ifc);
	cmdline.addOption ('v', "verbose", "Enable verbose mode", &options.verbose, true);
	cmdline.addOption ('r', "repeat", "CNT", "Send the file/frame CNT times", &options.repeat, true);
	cmdline.addOption ('d', "delay", "SECONDS", "Packet transmission is delayed SECONDS", &options.delay, true);
	cmdline.addOption ('a', "interactive", "Enable interactive mode", &options.interactive, true);
}

cTcpPump::~cTcpPump()
{
	// TODO Auto-generated destructor stub
}

int cTcpPump::execute (int argc, char* argv[])
{
	if (options.verbose)
		Console::SetPrintLevel(Console::Verbose);

	if (!argc)
	{
		Console::PrintError ("no packet data provided\n");
		return -2;
	}

	cInterface ifc (options.ifc);
	if (!ifc.open())
		return -1;

	int ret = 0;

	for (int n = 0; n < argc && !ret; n++)
	{
		packet_t packet;
		packet.packet = Converter::hexStringToBin (argv[n], &packet.packetLen);

		if (packet.packet)
			packets.push_back(packet);
		else
			ret = -3;
	}
	Console::PrintVerbose ("Sending %d packets, each delayed by %d seconds. Repeating %d times.\n\n", argc, options.delay, options.repeat);
	while (options.repeat-- && !ret)
	{
		for (const packet_t& p : packets)
		{
			if (options.delay)
				Console::PrintVerbose ("Waiting %d seconds\n", options.delay);
			System::Sleep (options.delay);
			if(!ifc.sendPacket (p.packet, p.packetLen))
			{
				ret = -4;
				break;
			}
		}
	}

	for (const packet_t& p : packets)
	{
		free ((void*)p.packet);
	}

	return ret;
}

int main(int argc, char* argv[])
{
	cTcpPump app ("tcppump -i IFC [OPTIONS] packets", "An Ethernet packet generator", "TODO description");
	return app.main (argc, argv);
}

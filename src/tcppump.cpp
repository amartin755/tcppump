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
#include "libnetnag/instructionparser.hpp"
#include "libnetnag/fileparser.hpp"
#include "libnetnag/ethernetpacket.hpp"

using namespace nn;

cTcpPump::cTcpPump(const char* name, const char* brief, const char* description)
: cCmdlineApp (name, brief, description)
{
	memset (&options, 0, sizeof(options));
	options.repeat = 1;
	options.delay  = 0;

	cmdline.addOption ('i', "interface", "IFC", "Interface name", &options.ifc);
	cmdline.addOption ('v', "verbose", "Enable verbose mode", &options.verbose, true);
	cmdline.addOption ('n', "repeat", "N", "Send the file/frame N times", &options.repeat, true);
	cmdline.addOption ('d', "delay", "SECONDS", "Packet transmission is delayed SECONDS", &options.delay, true);
	cmdline.addOption ('a', "interactive", "Enable interactive mode", &options.interactive, true);
	cmdline.addOption ('r', "raw", "Send raw packets without intepretation", &options.raw, true);
	cmdline.addOption ('s', "script", "Execute script file", &options.script, true);
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
		Console::PrintError (options.script ? "no script files provided\n": "no packet data provided\n");
		return -2;
	}

	cInterface ifc (options.ifc);
	if (!ifc.open())
		return -1;

	bool ok = options.script ? parseScripts (ifc.getMAC(), argc, argv) : parsePackets (ifc.getMAC(), argc, argv);
	if (!ok)
		return -2;

	Console::PrintVerbose ("Sending %d packets, each delayed by %d seconds. Repeating %d times.\n\n", packets.size(), options.delay, options.repeat);
	while (options.repeat--)
	{
		for (cEthernetPacket& p : packets)
		{
			if (options.delay)
				Console::PrintVerbose ("Waiting %d seconds\n", options.delay);
			System::Sleep (options.delay);
			if(!ifc.sendPacket (p.get(), p.getLength()))
			{
				return -4;
			}
		}
	}

	return 0;
}


bool cTcpPump::parsePackets (mac_t ownMac, int argc, char* argv[])
{
	for (int n = 0; n < argc; n++)
	{
		try
		{
			packets.emplace_back();
		}
		catch (...)
		{
			Console::PrintError ("Not enough memory\n");
			return false;
		}

		try
		{
			if (!options.raw)
				cInstructionParser (ownMac, 0).parse (argv[n], packets.back());
			else
				packets.back().setRaw (argv[n], strlen (argv[n]));
		}
		catch (ParseException &e)
		{
			Console::PrintError ("%s %s\n", e.what (), e.value ());
			return false;
		}
		catch (FormatException& e)
		{
			Console::PrintError ("%s %s\n", e.why (), e.value ());
			return false;
		}
	}
	return true;
}


bool cTcpPump::parseScripts (mac_t ownMac, int scriptsCnt, char* scripts[])
{
	FILE *fp;
	int len;
	cFileParser parser;

	do
	{
		if (scriptsCnt && ((fp = fopen (*scripts, "rt")) == NULL))
		{
			Console::PrintError ("Unable to open the script file %s.\n", *scripts);
			return false;
		}

		parser.init (fp, 0, ownMac, 0);

		do
		{
		//	printf ("capacity %d\n", packets.capacity());
			// allocate a new packet
			try
			{
				packets.emplace_back();
			}
			catch (...)
			{
				Console::PrintError ("Not enough memory\n");
				return false;
			}

		}while ((len = parser.parse (packets.back())) > 0);

		if (len == PARSE_ERROR)
			Console::PrintError ("%s %s\n", *scripts, parser.getLastError ());

		fclose (fp);

		if (len != EOF)
			return false;

		scripts++;

	}while (--scriptsCnt > 0);

	return true;
}


int main(int argc, char* argv[])
{
	cTcpPump app ("tcppump -i IFC [OPTIONS] packets", "An Ethernet packet generator", "TODO description");
	return app.main (argc, argv);
}

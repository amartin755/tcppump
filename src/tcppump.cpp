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
#include "sleep.hpp"
#include "getch.hpp"
#include "interface.hpp"
#include "dissector.hpp"
#include "libnetnag/converter.hpp"
#include "libnetnag/instructionparser.hpp"
#include "libnetnag/fileparser.hpp"
#include "libnetnag/ethernetpacket.hpp"

using namespace nn;

cTcpPump::cTcpPump(const char* name, const char* brief, const char* usage, const char* description)
: cCmdlineApp (name, brief, usage, description)
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
	mac_t ownMac;
	if (!ifc.getMAC(&ownMac))
	{
		Console::PrintError ("Could not determine mac address of interface.\n");
		return -1;
	}

	bool ok = options.script ? parseScripts (ownMac, argc, argv) : parsePackets (ownMac, argc, argv);
	if (!ok)
		return -2;

	if (!options.interactive)
	{
		Console::PrintVerbose ("Sending %d packets, each delayed by %d seconds. Repeating %d times.\n\n", packets.size(), options.delay, options.repeat);
		while (options.repeat--)
		{
			for (cEthernetPacket& p : packets)
			{
				if (!sendPacket (ifc, p))
					return -4;
			}
		}
	}
	else
	{
		// TODO
		// proof-of-concept for interactive-mode
		int defaultKeys[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
		int n = 0;
		for (cEthernetPacket& p : packets)
		{
			keyBindings.insert (std::pair<int, cEthernetPacket&>(defaultKeys[n++], p));
		}

		int key;

		while ((key = tcppump::getch ()) != EOF)
		{
			try
			{
				cEthernetPacket& p = keyBindings.at (key);
				if (!sendPacket (ifc, p))
					return -4;
			}
			catch (const std::out_of_range& e)
			{
				// key not found --> nothing to do
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
			cEthernetPacket packet;
			if (!options.raw)
				cInstructionParser (ownMac, 0).parse (argv[n], packet);
			else
				packet.setRaw (argv[n], strlen (argv[n]));

			packets.push_back (std::move(packet));
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
			// allocate a new packet
			try
			{
				cEthernetPacket packet;
				len = parser.parse (packet);
				if (len > 0)
					packets.push_back (std::move(packet));
			}
			catch (...)
			{
				Console::PrintError ("Not enough memory\n");
				return false;
			}

		}while (len > 0);

		fclose (fp);

		if (len == PARSE_ERROR)
			Console::PrintError ("%s %s\n", *scripts, parser.getLastError ());
		if (len != EOF)
			return false;

		scripts++;

	}while (--scriptsCnt > 0);

	return true;
}


bool cTcpPump::sendPacket (cInterface &ifc, cEthernetPacket &p)
{
	if (options.delay)
		Console::PrintVerbose ("Waiting %d seconds\n", options.delay);
	tcppump::Sleep (options.delay);
	if(!ifc.sendPacket (p.get(), p.getLength()))
	{
		Console::PrintError ("Could not send packet.\n");
		return false;
	}
	cDissector(p).dissect();

	return true;
}


int main(int argc, char* argv[])
{
	cTcpPump app ("tcppump", "An Ethernet packet generator", "tcppump -i IFC [OPTIONS] packets", "TODO description");
	return app.main (argc, argv);
}

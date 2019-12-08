/**
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2019 Andreas Martin (netnag@mailbox.org)
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

#include <cstdio>
#include <cassert>

#include "converter.hpp"
#include "console.hpp"
#include "timeval.hpp"
#include "cmdline.hpp"
#include "pcapfileio.hpp"
#include "instructionparser.hpp"


int main (void)
{
	nn::Console::SetPrintLevel(nn::Console::Debug);
	try
	{
		nn::Converter::unitTest ();
		cTimeval::unitTest ();
		cCmdline::unitTest ();
#if HAVE_PCAP
		cPcapFileIO::unitTest ("../src/test/readtest.pcap");
#endif
		cInstructionParser::unitTest ();
	}
	catch (...)
	{
		assert ("unhandled exception" == 0);
	}
	// every failure will lead to assert, thus if we see this output, all tests have passed
	printf ("\n --- unit tests finished successfully !!! --- \n");

	return 0;
}

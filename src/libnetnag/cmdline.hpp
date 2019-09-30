/**
 * NETNAG <https://github.com/amartin755/netnag>
 * Copyright (C) 2012-2016 Andreas Martin (netnag@mailbox.org)
 *
 * cmdline.hpp
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


#ifndef CMDLINE_HPP_
#define CMDLINE_HPP_

#include <vector>

typedef enum {ARG_NO, ARG_STRING, ARG_INT}arg_type;

typedef struct
{
	int         isSet;
	int         shortname;
	const char* longname;
	const char* argname;
	const char* description;
	bool        optional;
	bool        hasArg;
	bool        hasOptionalArg;
	arg_type type;
	// usage depends on the type
	void* arg;
	int*		pOptSet;
}argument;

class cCmdline
{
public:
	cCmdline (int argc, char* argv[]);
	cCmdline ();
	virtual ~cCmdline();

#ifdef WITH_UNITTESTS
	static void unitTest ();
#endif

	bool addOption (bool optional, char shortname, const char* longname, const char* description, int* isOptionSet,
			const char* argname = nullptr, arg_type type = ARG_NO, void* arg = nullptr, bool hasOptionalArg = false);

	bool parse (int* optind = 0);
	bool parse (int argc, char* argv[], int* optind = 0);
	void printOptions ();

private:
	int argc;
	char** argv;
	std::vector<argument> options;

	int findOption (int shortname);
};

#endif /* CMDLINE_HPP_ */

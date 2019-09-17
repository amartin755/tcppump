/**
 * NETNAG <https://github.com/amartin755/netnag>
 * Copyright (C) 2012-2016 Andreas Martin (netnag@mailbox.org)
 *
 * cmdlineapp.hpp
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


#ifndef CMDLINEAPP_HPP_
#define CMDLINEAPP_HPP_

#include "cmdline.hpp"
#include "console.hpp"

class cCmdlineApp
{
public:
	cCmdlineApp (const char* name, const char* brief, const char* usage, const char* description)
	{
			this->name = name;
			this->brief = brief;
			this->usage = usage;
			this->description = description;
			this->help = false;

			cmdline.addOption ('h', "help", "Display this text", &help, true);
	}
	virtual ~cCmdlineApp ()
	{
	}
	int main (int argc, char* argv[])
	{
		int index = 0;
		bool parseOk = cmdline.parse (argc, argv, &index);

		if (help)
		{
			printUsage ();
			return 0;
		}

		if (!parseOk)
		{
			nn::Console::PrintError ("try %s -h\n", argv[0]);
			return -1;
		}

		return this->execute (argc - index, &argv[index]);
	}

protected:
	virtual int execute (int argc, char* argv[]) = 0;
	void printUsage ()
	{
		const char* version = "";
#ifdef APP_VERSION
		version = " V" APP_VERSION;
#endif
		nn::Console::Print ("\n%s%s - %s\n\nUsage: %s\n\nOptions:\n", name, version, brief, usage);
		cmdline.printOptions ();
		nn::Console::Print ("\n%s\n\n", description);
	}

	// adds integer option with argument
	bool addCmdLineOption (char shortname, const char* longname, const char* argname, const char* description, int* arg, bool optional = false)
	{
		return cmdline.addOption (shortname, longname, argname, description, arg, optional);
	}
	// adds string option with argument
	bool addCmdLineOption (char shortname, const char* longname, const char* argname, const char* description, const char** arg, bool optional = false)
	{
		return cmdline.addOption (shortname, longname, argname, description, arg, optional);
	}
	// adds boolean option without argument
	bool addCmdLineOption (char shortname, const char* longname, const char* description, bool* optSet, bool optional = false)
	{
		return cmdline.addOption (shortname, longname, description, optSet, optional);
	}
	// adds boolean option without argument, returns how often a option was set (e.g. -vvv --> optSet = 3)
	bool addCmdLineOption (char shortname, const char* longname, const char* description, int* optSet, bool optional)
	{
		return cmdline.addOption (shortname, longname, description, optSet, optional);
	}

private:
	const char* name;
	const char* brief;
	const char* usage;
	const char* description;
	bool help;
	cCmdline cmdline;
};

#endif /* CMDLINE_HPP_ */

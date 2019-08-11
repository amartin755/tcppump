/**
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2016 Andreas Martin (netnag@mailbox.org)
 *
 * cmdline.cpp
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

#include <cassert>
#include <cstring>
#include <cstdlib>
#include <getopt.h>

#include "cmdline.hpp"
#include "console.hpp"


cCmdline::cCmdline (int argc, char* argv[])
{
	this->argc = argc;
	this->argv = argv;
}

cCmdline::cCmdline ()
{
	this->argc = 0;
	this->argv = NULL;
}

cCmdline::~cCmdline ()
{
	// TODO Auto-generated destructor stub
}

bool cCmdline::addOption (char shortname, const char* longname, const char* argname, const char* description, int* arg, bool optional)
{
	return addOption (shortname, longname, argname, description, ARG_INT, (void*)arg, true, optional);
}


bool cCmdline::addOption (char shortname, const char* longname, const char* argname, const char* description, const char** arg, bool optional)
{
	return addOption (shortname, longname, argname, description, ARG_STRING, (void*)arg, true, optional);
}


bool cCmdline::addOption (char shortname, const char* longname, const char* description, bool* optSet, bool optional)
{
	return addOption (shortname, longname, NULL, description, ARG_BOOL, (void*)optSet, false, optional);
}


bool cCmdline::parse (int argc, char* argv[], int* optind)
{
	this->argc = argc;
	this->argv = argv;

	return parse (optind);
}


bool cCmdline::parse (int* optind)
{
	bool ret = true;
	char* shortopts = new (std::nothrow) char[options.size() * 2 + 1];
	char* pShort = shortopts;
	if (!shortopts)
	{
		nn::Console::PrintError ("Not enough memory\n");
		return false;
	}

	struct option *longopts = new (std::nothrow) struct option[options.size() + 1];
	struct option *pLong = longopts;
	if (!longopts)
	{
		delete[] shortopts;
		nn::Console::PrintError ("Not enough memory\n");
		return false;
	}

	for (unsigned n = 0; n < options.size (); n++)
	{
		if (options.at (n).shortname)
		{
			*pShort++ = options.at (n).shortname;
			if (options.at (n).hasArg)
			{
				*pShort++ = ':';
			}
		}
		if (options.at (n).longname)
		{
			pLong->name    = options.at (n).longname;
			pLong->has_arg = options.at (n).hasArg ? required_argument : no_argument;
			pLong->flag    = NULL;
			pLong->val     = options.at (n).shortname;
			pLong++;
		}
	}

	pShort = '\0';
	pLong->name    = NULL;
	pLong->has_arg = 0;
	pLong->flag    = NULL;
	pLong->val     = 0;

	::optind = 1;

	while (::optind < argc && ret)
	{
		int index = 0;
		int result = getopt_long (argc, argv, shortopts, longopts, &index);

		if (result == -1)
			break;
		else if (result == '?')
		{
			int option = findOption (optopt);
			if (option >= 0)
			{
				nn::Console::PrintError ("Option -%c requires an argument.\n", optopt);
			}
			else
			{
				nn::Console::PrintError ("Unknown option `-%c'.\n", optopt);
			}
			ret = false;
		}
		else
		{
			int option = findOption (result);
			if (option >= 0)
			{
				switch (options.at (option).type)
				{
				case ARG_STRING:
					*((char**)options.at (option).arg) = optarg;
					break;
				case ARG_INT:
					*((int*)options.at (option).arg) = (int)strtol (optarg, NULL, 0);
					break;
				case ARG_BOOL:
					if (options.at (option).hasArg)
						*((bool*)options.at (option).arg) = strtol (optarg, NULL, 0) ? true : false;
					else
						*((bool*)options.at (option).arg) = true;
					break;
				}
				options.at (option).isSet = true;
			}
			else
			{
				assert ("getopt returned unexpected value" == 0);
			}
		}
	}

	// check if all mandatory options are present
	for (unsigned n = 0; ret && n < options.size (); n++)
	{
		if (!options.at (n).optional && !options.at (n).isSet)
		{
			nn::Console::PrintError ("mandatory option -%c --%s not set\n", options.at (n).shortname, options.at (n).longname);
			ret = false;
		}
	}

	delete[] shortopts;
	delete[] longopts;

	if (optind)
		*optind = ::optind;

	return ret;
}


void cCmdline::printOptions ()
{
	//FIXME still incomplete
	for (unsigned n = 0; n < options.size (); n++)
	{
		nn::Console::Print ("-%c ", options.at (n).shortname);
		if (options.at (n).hasArg)
			nn::Console::Print ("%s ", options.at (n).argname);
		if (options.at (n).longname)
		{
			nn::Console::Print ("--%s", options.at (n).longname);
			if (options.at (n).hasArg)
				nn::Console::Print ("=%s ", options.at (n).argname);
			nn::Console::Print ("\n\t");
		}
		if (options.at (n).description)
			nn::Console::Print ("%s", options.at (n).description);
		nn::Console::Print ("\n");
	}
}


bool cCmdline::addOption (char shortname, const char* longname, const char* argname, const char* description, arg_type type, void* arg, bool hasArg, bool optional)
{
	// NOTE 'longname', 'description' and 'argname' are assumed to be static!

	int len = strlen (longname);
	if (len <= 1)
		return false;

	argument a = {0};

	a.shortname   = shortname;
	a.longname    = longname;
	a.argname     = argname;
	a.description = description;
	a.type        = type;
	a.arg         = arg;
	a.optional    = optional;
	a.hasArg      = hasArg;
	a.isSet       = false;

	options.push_back (a);

	return true;
}


int cCmdline::findOption (char shortname)
{
	for (unsigned n = 0; n < options.size (); n++)
	{
		if (options.at (n).shortname == shortname)
			return n;
	}
	return -1;
}


void cCmdline::unitTest ()
{
	// parsing rules:
	// - short options without args -a -b -c == -abc
	// - short options with args -aARG == -a ARG
	// - long options with args --aaa=ARG == --aaa ARG
	// - parsing stops when the first non-option argument is detected

	bool boolarg1 = false;
    bool boolarg2 = false;
    int  intarg1 = -1;
    int  intarg2 = -1;
    const char* stringarg1 = NULL;
    const char* stringarg2 = NULL;

    {
		char* argv[] = {"unittest1", "-aAAA", "-b", "-cCCC", "-d"};
		int argc = 5;
		boolarg1 = boolarg2 = false;
		stringarg1 = stringarg2 = NULL;

		cCmdline obj(argc, (char**)argv);

		assert (!obj.addOption ('a', "a", "ARG", "illegal option", ARG_BOOL, &boolarg1, true, false));
		assert (obj.addOption ('a', "arga", "ARG", "mandatory option with args", ARG_STRING, &stringarg1, true, false));
		assert (obj.addOption ('b', "argb", "ARG", "mandatory option without args (makes no sense)", ARG_BOOL, &boolarg1, false, false));
		assert (obj.addOption ('c', "argc", "ARG", "optional option with args", ARG_STRING, &stringarg2, true, true));
		assert (obj.addOption ('d', "argd", "ARG", "optional option without args", ARG_BOOL, &boolarg2, false, true));

		assert (obj.parse ());
		assert (stringarg1);
		assert (!strcmp ("AAA", stringarg1));
		assert (boolarg1);
		assert (stringarg2);
		assert (!strcmp ("CCC", stringarg2));
		assert (boolarg2);
    }
    {
		char* argv[] = {"unittest2", "-a", "AAA", "-b", "-c", "CCC", "-d"};
		int argc = 7;
		boolarg1 = boolarg2 = false;
		stringarg1 = stringarg2 = NULL;

		cCmdline obj(argc, (char**)argv);

		assert (obj.addOption ('a', "arga", "ARG", "mandatory option with args", ARG_STRING, &stringarg1, true, false));
		assert (obj.addOption ('b', "argb", "ARG", "mandatory option without args (makes no sense)", ARG_BOOL, &boolarg1, false, false));
		assert (obj.addOption ('c', "argc", "ARG", "optional option with args", ARG_STRING, &stringarg2, true, true));
		assert (obj.addOption ('d', "argd", "ARG", "optional option without args", ARG_BOOL, &boolarg2, false, true));

		assert (obj.parse ());
		assert (stringarg1);
		assert (!strcmp ("AAA", stringarg1));
		assert (boolarg1);
		assert (stringarg2);
		assert (!strcmp ("CCC", stringarg2));
		assert (boolarg2);
    }
    {
		char* argv[] = {"unittest3", "--arga=AAA", "--argb", "--argc=CCC", "--argd"};
		int argc = 5;
		boolarg1 = boolarg2 = false;
		stringarg1 = stringarg2 = NULL;

		cCmdline obj(argc, (char**)argv);

		assert (obj.addOption ('a', "arga", "ARG", "mandatory option with args", ARG_STRING, &stringarg1, true, false));
		assert (obj.addOption ('b', "argb", "ARG", "mandatory option without args (makes no sense)", ARG_BOOL, &boolarg1, false, false));
		assert (obj.addOption ('c', "argc", "ARG", "optional option with args", ARG_STRING, &stringarg2, true, true));
		assert (obj.addOption ('d', "argd", "ARG", "optional option without args", ARG_BOOL, &boolarg2, false, true));

		assert (obj.parse ());
		assert (stringarg1);
		assert (!strcmp ("AAA", stringarg1));
		assert (boolarg1);
		assert (stringarg2);
		assert (!strcmp ("CCC", stringarg2));
		assert (boolarg2);
    }
    {
		char* argv[] = {"unittest4", "--arga", "AAA", "--argb", "--argc", "CCC", "--argd"};
		int argc = 7;
		boolarg1 = boolarg2 = false;
		stringarg1 = stringarg2 = NULL;

		cCmdline obj(argc, (char**)argv);

		assert (obj.addOption ('a', "arga", "ARG", "mandatory option with args", ARG_STRING, &stringarg1, true, false));
		assert (obj.addOption ('b', "argb", "ARG", "mandatory option without args (makes no sense)", ARG_BOOL, &boolarg1, false, false));
		assert (obj.addOption ('c', "argc", "ARG", "optional option with args", ARG_STRING, &stringarg2, true, true));
		assert (obj.addOption ('d', "argd", "ARG", "optional option without args", ARG_BOOL, &boolarg2, false, true));

		assert (obj.parse ());
		assert (stringarg1);
		assert (!strcmp ("AAA", stringarg1));
		assert (boolarg1);
		assert (stringarg2);
		assert (!strcmp ("CCC", stringarg2));
		assert (boolarg2);
    }
    {
		char* argv[] = {"unittest5", "-a", "-b", "-cCCC", "-d"};
		int argc = 5;
		boolarg1 = boolarg2 = false;
		stringarg1 = stringarg2 = NULL;

		cCmdline obj(argc, (char**)argv);

		assert (obj.addOption ('a', "arga", "ARG", "mandatory option with args", &stringarg1));
		assert (obj.addOption ('b', "argb", "mandatory option without args (makes no sense)", &boolarg1));
		assert (obj.addOption ('c', "argc", "ARG", "optional option with args", &stringarg2, true));
		assert (obj.addOption ('d', "argd", "optional option without args", &boolarg2, true));

		assert (!obj.parse ());
		assert (stringarg1);
		assert (!strcmp ("-b", stringarg1));
		assert (!boolarg1);
		assert (stringarg2);
		assert (!strcmp ("CCC", stringarg2));
		assert (boolarg2);
    }
    {
		char* argv[] = {"unittest6", "-b", "-cCCC"};
		int argc = 3;
		boolarg1 = boolarg2 = false;
		stringarg1 = stringarg2 = NULL;

		cCmdline obj(argc, (char**)argv);

		assert (obj.addOption ('a', "arga", "ARG", "mandatory option with args", &stringarg1));
		assert (obj.addOption ('b', "argb", "mandatory option without args (makes no sense)", &boolarg1));
		assert (obj.addOption ('c', "argc", "ARG", "optional option with args", &stringarg2, true));
		assert (obj.addOption ('d', "argd", "optional option without args", &boolarg2, true));

		assert (!obj.parse ());
		assert (!stringarg1);
		assert (boolarg1);
		assert (stringarg2);
		assert (!strcmp ("CCC", stringarg2));
		assert (!boolarg2);
    }
    {
		char* argv[] = {"unittest7", "-a3", "-b"};
		int argc = 3;
		boolarg1 = boolarg2 = false;
		stringarg1 = stringarg2 = NULL;
		intarg1 = intarg2 = -1;

		cCmdline obj(argc, (char**)argv);

		assert (obj.addOption ('a', "arga", "ARG", "optional option with args", ARG_BOOL, &boolarg1, true, true));
		assert (obj.addOption ('b', "argb", "ARG", "optional option without args", ARG_BOOL, &boolarg2, false, true));

		assert (obj.parse ());
		assert (boolarg1);
		assert (boolarg2);
    }
    {
		char* argv[] = {"unittest8", "-a0", "-b"};
		int argc = 3;
		boolarg1 = true;
		boolarg2 = false;
		stringarg1 = stringarg2 = NULL;
		intarg1 = intarg2 = -1;

		cCmdline obj(argc, (char**)argv);

		assert (obj.addOption ('a', "arga", "ARG", "optional option with args", ARG_BOOL, &boolarg1, true, true));
		assert (obj.addOption ('b', "argb", "ARG", "optional option without args", ARG_BOOL, &boolarg2, false, true));

		assert (obj.parse ());
		assert (!boolarg1);
		assert (boolarg2);
    }
    {
		char* argv[] = {"unittest9", "-a0xa", "-b10"};
		int argc = 3;
		boolarg1 = boolarg2 = false;
		stringarg1 = stringarg2 = NULL;
		intarg1 = intarg2 = -1;

		cCmdline obj(argc, (char**)argv);

		assert (obj.addOption ('a', "arga", "ARG", "optional option with args", &intarg1, true));
		assert (obj.addOption ('b', "argb", "ARG", "optional option with args", &intarg2, true));

		assert (obj.parse ());
		assert (intarg1 == 10);
		assert (intarg2 == 10);
    }
    {
		char* argv[] = {"unittest10", "-a", "0xa", "-b", "10"};
		int argc = 5;
		boolarg1 = boolarg2 = false;
		stringarg1 = stringarg2 = NULL;
		intarg1 = intarg2 = -1;

		cCmdline obj(argc, (char**)argv);

		assert (obj.addOption ('a', "arga", "ARG", "optional option with args", &intarg1, true));
		assert (obj.addOption ('b', "argb", "ARG", "optional option with args", &intarg2, true));

		assert (obj.parse ());
		assert (intarg1 == 10);
		assert (intarg2 == 10);
    }
    {
		char* argv[] = {"unittest11", "--arga=0xa", "--argb=10"};
		int argc = 3;
		boolarg1 = boolarg2 = false;
		stringarg1 = stringarg2 = NULL;
		intarg1 = intarg2 = -1;

		cCmdline obj(argc, (char**)argv);

		assert (obj.addOption ('a', "arga", "ARG", "optional option with args", &intarg1, true));
		assert (obj.addOption ('b', "argb", "ARG", "optional option with args", &intarg2, true));

		assert (obj.parse ());
		assert (intarg1 == 10);
		assert (intarg2 == 10);
    }
    {
		char* argv[] = {"unittest12", "--arga", "0xa", "--argb", "10"};
		int argc = 5;
		boolarg1 = boolarg2 = false;
		stringarg1 = stringarg2 = NULL;
		intarg1 = intarg2 = -1;

		cCmdline obj(argc, (char**)argv);

		assert (obj.addOption ('a', "arga", "ARG", "optional option with args", &intarg1, true));
		assert (obj.addOption ('b', "argb", "ARG", "optional option with args", &intarg2, true));

		assert (obj.parse ());
		assert (intarg1 == 10);
		assert (intarg2 == 10);
    }
    {
		char* argv[] = {"unittest13", "--arga=0xa", "--argb", "10", "--argc=CCC", "--argd", "DDD", "ABCD", "EFGH"};
		int argc = 9;
		boolarg1 = boolarg2 = false;
		stringarg1 = stringarg2 = NULL;
		intarg1 = intarg2 = -1;
		int index = 0;

		cCmdline obj(argc, (char**)argv);

		assert (obj.addOption ('a', "arga", "ARG", "optional option with args", &intarg1, true));
		assert (obj.addOption ('b', "argb", "ARG", "optional option with args", &intarg2, true));
		assert (obj.addOption ('c', "argc", "ARG", "optional option with args", &stringarg1, true));
		assert (obj.addOption ('d', "argd", "ARG", "optional option with args", &stringarg2, true));

		assert (obj.parse (&index));
		assert (intarg1 == 10);
		assert (intarg2 == 10);
		assert (stringarg1);
		assert (!strcmp ("CCC", stringarg1));
		assert (stringarg2);
		assert (!strcmp ("DDD", stringarg2));
		assert (index == 7);
    }
}

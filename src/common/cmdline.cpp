/**
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2020 Andreas Martin (netnag@mailbox.org)
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
#include <cstdlib>

#include "ketopt.h"

#include "cmdline.hpp"

#include "bug.h"
#include "console.hpp"


const int NO_SHORTNAME = 0x100;


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



bool cCmdline::parse (int argc, char* argv[], int* optind)
{
    this->argc = argc;
    this->argv = argv;

    return parse (optind);
}


bool cCmdline::parse (int* optind)
{
    ketopt_t opt = KETOPT_INIT;
    bool ret = true;
    char* shortopts = new (std::nothrow) char[options.size() * 2 + 1];
    char* pShort = shortopts;
    if (!shortopts)
    {
        Console::PrintError ("Not enough memory\n");
        return false;
    }

    ko_longopt_t *longopts = new (std::nothrow) ko_longopt_t[options.size() + 1];
    ko_longopt_t *pLong = longopts;
    if (!longopts)
    {
        delete[] shortopts;
        Console::PrintError ("Not enough memory\n");
        return false;
    }

    for (unsigned n = 0; n < options.size (); n++)
    {
        if (options.at (n).shortname < NO_SHORTNAME)
        {
            *pShort++ = (char)options.at (n).shortname;
            if (options.at (n).hasArg)
            {
                *pShort++ = ':';
            }
        }
        if (options.at (n).longname)
        {
            pLong->name    = (char*)options.at (n).longname;
            pLong->has_arg = options.at (n).hasArg ? ko_required_argument : ko_no_argument;
            if (options.at (n).hasOptionalArg && options.at (n).hasArg)
                pLong->has_arg = ko_optional_argument;
            pLong->val     = options.at (n).shortname;
            pLong++;
        }
    }

    *pShort        = '\0';
    pLong->name    = NULL;
    pLong->has_arg = 0;
    pLong->val     = 0;


    int result;
    while ((result = ketopt (&opt, argc, argv, 1, shortopts, longopts)) >= 0 && ret)
    {
        if (result == '?')
        {
            Console::PrintError ("Unknown option `%s'.\n", opt.ind - 1 <= argc ? argv[opt.ind - 1] : "???");
            ret = false;
        }
        else if (result == ':')
        {
            Console::PrintError ("Option %s requires an argument.\n", opt.ind - 1 <= argc ? argv[opt.ind - 1] : "???");
            ret = false;
        }
        else
        {
            int option = findOption (opt.opt);
            if (option >= 0)
            {
                argument &o = options.at (option);
                if (o.hasArg && opt.arg)
                {
                    if (o.type == ARG_STRING)
                        *((char**)o.arg) = opt.arg;
                    if (o.type == ARG_INT)
                        *((int*)o.arg) = (int)strtol (opt.arg, NULL, 0);
                }
                o.isSet++;
            }
            else
            {
                BUG ("getopt returned unexpected value");
            }
        }
    }

    // first check whether options like --help or --version are set. If yes, we don't fail if mandatory options are missing
    bool enforceMandatoryOptions = true;
    for (const auto &currOpt : options)
    {
        if (currOpt.dontFailIfSet && currOpt.isSet)
        {
            enforceMandatoryOptions = false;
            break;
        }
    }
    for (const auto &currOpt : options)
    {
        // check if all mandatory options are present
        if (enforceMandatoryOptions && !currOpt.optional && !currOpt.isSet)
        {
            Console::PrintError ("mandatory option -%c --%s not set\n", currOpt.shortname, currOpt.longname);
            ret = false;
        }

        // return how often option was present
        if (currOpt.pOptSet)
            *(currOpt.pOptSet) = currOpt.isSet;
    }

    delete[] shortopts;
    delete[] longopts;

    if (optind)
        *optind = opt.ind;

    return ret;
}


void cCmdline::printOptions ()
{
    //FIXME still incomplete
    for (unsigned n = 0; n < options.size (); n++)
    {
        if (options.at (n).shortname < NO_SHORTNAME)
        {
            Console::Print ("-%c ", options.at (n).shortname);
            if (options.at (n).hasArg)
                Console::Print ("%s ", options.at (n).argname);
        }
        if (options.at (n).longname)
        {
            Console::Print ("--%s", options.at (n).longname);
            if (options.at (n).hasArg)
            {
                if (options.at (n).hasOptionalArg)
                    Console::Print ("[=%s] ", options.at (n).argname);
                else
                    Console::Print ("=%s ", options.at (n).argname);
            }

            Console::Print ("\n\t");
        }
        if (options.at (n).description)
            Console::Print ("%s", options.at (n).description);
        Console::Print ("\n");
    }
}


// NOTE 'longname', 'description' and 'argname' are assumed to be static!
bool cCmdline::addOption (bool optional, char shortname, const char* longname, const char* description, int* isOptionSet,
        const char* argname, arg_type type, void* arg, bool hasOptionalArg, bool dontFailIfSet)
{
    if (hasOptionalArg && shortname)
        BUG ("optional arguments are only possible with long options");
    if (!shortname && !longname)
        BUG ("either shortname or longname must be != null");

    if (longname)
    {
        size_t len = strlen (longname);
        if (len <= 1)
            return false;
    }

    argument a;
    memset (&a, 0, sizeof (a));

    a.optional       = optional;
    a.pOptSet        = isOptionSet;
    a.shortname      = shortname ? shortname : NO_SHORTNAME + (int)options.size();
    a.longname       = longname;
    a.description    = description;
    a.dontFailIfSet  = dontFailIfSet;
    if (argname) // option has argument?
    {
        a.hasArg         = true;
        a.argname        = argname;
        a.type           = type;
        a.arg            = arg;
        a.hasOptionalArg = hasOptionalArg;
    }
    options.push_back (a);

    return true;
}


int cCmdline::findOption (int shortname)
{
    for (unsigned n = 0; n < options.size (); n++)
    {
        if (options.at (n).shortname == shortname)
            return n;
    }
    return -1;
}

#ifdef WITH_UNITTESTS
void cCmdline::unitTest ()
{
    Console::PrintDebug("-- " __FILE__ " --\n");

    // parsing rules:
    // - short options without args -a -b -c == -abc
    // - short options with args -aARG == -a ARG
    // - long options with args --aaa=ARG == --aaa ARG
    // - parsing stops when the first non-option argument is detected

    int  intarg1 = -1;
    int  intarg2 = -1;
    const char* stringarg1 = NULL;
    const char* stringarg2 = NULL;
    int  isset1 = -1;
    int  isset2 = -1;
    int  isset3 = -1;
    int  isset4 = -1;

    {
        char* argv[] = {"unittest1", "-aAAA", "-b", "-cCCC", "-d"};
        int argc = 5;
        stringarg1 = stringarg2 = NULL;
        isset1 = isset2 = isset3 = isset4 = -1;

        cCmdline obj(argc, (char**)argv);

        BUG_ON (obj.addOption (false, 'a', "arga", "mandatory option with args", &isset1, "ARG", ARG_STRING, &stringarg1));
        BUG_ON (obj.addOption (false, 'b', "argb", "mandatory option without args (makes no sense)", &isset2));
        BUG_ON (obj.addOption (true, 'c', "argc", "optional option with args", &isset3, "ARG", ARG_STRING, &stringarg2));
        BUG_ON (obj.addOption (true, 'd', "argd", "optional option without args", &isset4));

        BUG_ON (obj.parse ());
        BUG_ON (isset1 == 1);
        BUG_ON (stringarg1);
        BUG_ON (!strcmp ("AAA", stringarg1));
        BUG_ON (isset2 == 1);
        BUG_ON (isset3 == 1);
        BUG_ON (stringarg2);
        BUG_ON (!strcmp ("CCC", stringarg2));
        BUG_ON (isset4 == 1);
    }
    {
        char* argv[] = {"unittest2", "-a", "AAA", "-b", "-c", "CCC", "-d"};
        int argc = 7;
        isset1 = isset2 = isset3 = isset4 = -1;
        stringarg1 = stringarg2 = NULL;

        cCmdline obj(argc, (char**)argv);

        BUG_ON (obj.addOption (false, 'a', "arga", "mandatory option with args", &isset1, "ARG", ARG_STRING, &stringarg1));
        BUG_ON (obj.addOption (false, 'b', "argb", "mandatory option without args (makes no sense)", &isset2));
        BUG_ON (obj.addOption (true, 'c', "argc", "optional option with args", &isset3, "ARG", ARG_STRING, &stringarg2));
        BUG_ON (obj.addOption (true, 'd', "argd", "optional option without args", &isset4));

        BUG_ON (obj.parse ());
        BUG_ON (isset1 == 1);
        BUG_ON (stringarg1);
        BUG_ON (!strcmp ("AAA", stringarg1));
        BUG_ON (isset2 == 1);
        BUG_ON (isset3 == 1);
        BUG_ON (stringarg2);
        BUG_ON (!strcmp ("CCC", stringarg2));
        BUG_ON (isset4 == 1);
    }
    {
        char* argv[] = {"unittest3", "--arga=AAA", "--argb", "--argc=CCC", "--argd"};
        int argc = 5;
        isset1 = isset2 = isset3 = isset4 = -1;
        stringarg1 = stringarg2 = NULL;

        cCmdline obj(argc, (char**)argv);

        BUG_ON (obj.addOption (false, 'a', "arga", "mandatory option with args", &isset1, "ARG", ARG_STRING, &stringarg1));
        BUG_ON (obj.addOption (false, 'b', "argb", "mandatory option without args (makes no sense)", &isset2));
        BUG_ON (obj.addOption (true, 'c', "argc", "optional option with args", &isset3, "ARG", ARG_STRING, &stringarg2));
        BUG_ON (obj.addOption (true, 'd', "argd", "optional option without args", &isset4));

        BUG_ON (obj.parse ());
        BUG_ON (isset1 == 1);
        BUG_ON (stringarg1);
        BUG_ON (!strcmp ("AAA", stringarg1));
        BUG_ON (isset2 == 1);
        BUG_ON (isset3 == 1);
        BUG_ON (stringarg2);
        BUG_ON (!strcmp ("CCC", stringarg2));
        BUG_ON (isset4 == 1);
    }
    {
        char* argv[] = {"unittest4", "--arga", "AAA", "--argb", "--argc", "CCC", "--argd"};
        int argc = 7;
        isset1 = isset2 = isset3 = isset4 = -1;
        stringarg1 = stringarg2 = NULL;

        cCmdline obj(argc, (char**)argv);

        BUG_ON (obj.addOption (false, 'a', "arga", "mandatory option with args", &isset1, "ARG", ARG_STRING, &stringarg1));
        BUG_ON (obj.addOption (false, 'b', "argb", "mandatory option without args (makes no sense)", &isset2));
        BUG_ON (obj.addOption (true, 'c', "argc", "optional option with args", &isset3, "ARG", ARG_STRING, &stringarg2));
        BUG_ON (obj.addOption (true, 'd', "argd", "optional option without args", &isset4));

        BUG_ON (obj.parse ());
        BUG_ON (isset1 == 1);
        BUG_ON (stringarg1);
        BUG_ON (!strcmp ("AAA", stringarg1));
        BUG_ON (isset2 == 1);
        BUG_ON (isset3 == 1);
        BUG_ON (stringarg2);
        BUG_ON (!strcmp ("CCC", stringarg2));
        BUG_ON (isset4 == 1);
    }
    {
        char* argv[] = {"unittest5", "-a", "-b", "-cCCC", "-d"};
        int argc = 5;
        isset1 = isset2 = isset3 = isset4 = -1;
        stringarg1 = stringarg2 = NULL;

        cCmdline obj(argc, (char**)argv);

        BUG_ON (obj.addOption (false, 'a', "arga", "mandatory option with args", &isset1, "ARG", ARG_STRING, &stringarg1));
        BUG_ON (obj.addOption (false, 'b', "argb", "mandatory option without args (makes no sense)", &isset2));
        BUG_ON (obj.addOption (true, 'c', "argc", "optional option with args", &isset3, "ARG", ARG_STRING, &stringarg2));
        BUG_ON (obj.addOption (true, 'd', "argd", "optional option without args", &isset4));

        BUG_ON (!obj.parse ());
        BUG_ON (isset1 == 1);
        BUG_ON (stringarg1);
        BUG_ON (!strcmp ("-b", stringarg1));
        BUG_ON (isset2 == 0);
        BUG_ON (isset3 == 1);
        BUG_ON (stringarg2);
        BUG_ON (!strcmp ("CCC", stringarg2));
        BUG_ON (isset4 == 1);
    }
    {
        char* argv[] = {"unittest6", "-b", "-cCCC"};
        int argc = 3;
        isset1 = isset2 = isset3 = isset4 = -1;
        stringarg1 = stringarg2 = NULL;

        cCmdline obj(argc, (char**)argv);

        BUG_ON (obj.addOption (false, 'a', "arga", "mandatory option with args", &isset1, "ARG", ARG_STRING, &stringarg1));
        BUG_ON (obj.addOption (false, 'b', "argb", "mandatory option without args (makes no sense)", &isset2));
        BUG_ON (obj.addOption (true, 'c', "argc", "optional option with args", &isset3, "ARG", ARG_STRING, &stringarg2));
        BUG_ON (obj.addOption (true, 'd', "argd", "optional option without args", &isset4));

        BUG_ON (!obj.parse ());
        BUG_ON (isset1 == 0);
        BUG_ON (isset2 == 1);
        BUG_ON (isset3 == 1);
        BUG_ON (isset4 == 0);
        BUG_ON (!stringarg1);
        BUG_ON (stringarg2);
        BUG_ON (!strcmp ("CCC", stringarg2));
    }
    {
        char* argv[] = {"unittest7", "-a", "-b"};
        int argc = 3;
        isset1 = isset2 = isset3 = isset4 = -1;
        stringarg1 = stringarg2 = NULL;
        intarg1 = intarg2 = -1;

        cCmdline obj(argc, (char**)argv);

        BUG_ON (obj.addOption (true, 'a', "arga", "optional option without args", &isset1));
        BUG_ON (obj.addOption (true, 'b', "argb", "optional option without args", &isset2));

        BUG_ON (obj.parse ());
        BUG_ON (isset1 == 1);
        BUG_ON (isset2 == 1);
    }
    {
        char* argv[] = {"unittest8", "-b"};
        int argc = 2;
        isset1 = isset2 = isset3 = isset4 = -1;
        stringarg1 = stringarg2 = NULL;
        intarg1 = intarg2 = -1;

        cCmdline obj(argc, (char**)argv);

        BUG_ON (obj.addOption (true, 'a', "arga", "optional option without args", &isset1));
        BUG_ON (obj.addOption (true, 'b', "argb", "optional option without args", &isset2));

        BUG_ON (obj.parse ());
        BUG_ON (isset1 == 0);
        BUG_ON (isset2 == 1);
    }
    {
        char* argv[] = {"unittest9", "-a0xa", "-b10"};
        int argc = 3;
        isset1 = isset2 = isset3 = isset4 = -1;
        stringarg1 = stringarg2 = NULL;
        intarg1 = intarg2 = -1;

        cCmdline obj(argc, (char**)argv);

        BUG_ON (obj.addOption (true, 'a', "arga", "optional option with args", &isset1, "ARG", ARG_INT, &intarg1));
        BUG_ON (obj.addOption (true, 'b', "argb", "optional option with args", &isset2, "ARG", ARG_INT, &intarg2));

        BUG_ON (obj.parse ());
        BUG_ON (isset1 == 1);
        BUG_ON (isset2 == 1);
        BUG_ON (intarg1 == 10);
        BUG_ON (intarg2 == 10);
    }
    {
        char* argv[] = {"unittest10", "-a", "0xa", "-b", "10"};
        int argc = 5;
        isset1 = isset2 = isset3 = isset4 = -1;
        stringarg1 = stringarg2 = NULL;
        intarg1 = intarg2 = -1;

        cCmdline obj(argc, (char**)argv);

        BUG_ON (obj.addOption (true, 'a', "arga", "optional option with args", &isset1, "ARG", ARG_INT, &intarg1));
        BUG_ON (obj.addOption (true, 'b', "argb", "optional option with args", &isset2, "ARG", ARG_INT, &intarg2));

        BUG_ON (obj.parse ());
        BUG_ON (isset1 == 1);
        BUG_ON (isset2 == 1);
        BUG_ON (intarg1 == 10);
        BUG_ON (intarg2 == 10);
    }
    {
        char* argv[] = {"unittest11", "--arga=0xa", "--argb=10"};
        int argc = 3;
        isset1 = isset2 = isset3 = isset4 = -1;
        stringarg1 = stringarg2 = NULL;
        intarg1 = intarg2 = -1;

        cCmdline obj(argc, (char**)argv);

        BUG_ON (obj.addOption (true, 'a', "arga", "optional option with args", &isset1, "ARG", ARG_INT, &intarg1));
        BUG_ON (obj.addOption (true, 'b', "argb", "optional option with args", &isset2, "ARG", ARG_INT, &intarg2));

        BUG_ON (obj.parse ());
        BUG_ON (isset1 == 1);
        BUG_ON (isset2 == 1);
        BUG_ON (intarg1 == 10);
        BUG_ON (intarg2 == 10);
    }
    {
        char* argv[] = {"unittest12", "--arga", "0xa", "--argb", "10"};
        int argc = 5;
        isset1 = isset2 = isset3 = isset4 = -1;
        stringarg1 = stringarg2 = NULL;
        intarg1 = intarg2 = -1;

        cCmdline obj(argc, (char**)argv);

        BUG_ON (obj.addOption (true, 'a', "arga", "optional option with args", &isset1, "ARG", ARG_INT, &intarg1));
        BUG_ON (obj.addOption (true, 'b', "argb", "optional option with args", &isset2, "ARG", ARG_INT, &intarg2));

        BUG_ON (obj.parse ());
        BUG_ON (isset1 == 1);
        BUG_ON (isset2 == 1);
        BUG_ON (intarg1 == 10);
        BUG_ON (intarg2 == 10);
    }
    {
        char* argv[] = {"unittest13", "--arga=0xa", "--argb", "10", "--argc=CCC", "--argd", "DDD", "ABCD", "EFGH"};
        int argc = 9;
        isset1 = isset2 = isset3 = isset4 = -1;
        stringarg1 = stringarg2 = NULL;
        intarg1 = intarg2 = -1;
        int index = 0;

        cCmdline obj(argc, (char**)argv);

        BUG_ON (obj.addOption (true, 'a', "arga", "optional option with args", &isset1, "ARG", ARG_INT, &intarg1));
        BUG_ON (obj.addOption (true, 'b', "argb", "optional option with args", &isset2, "ARG", ARG_INT, &intarg2));
        BUG_ON (obj.addOption (true, 'c', "argc", "optional option with args", &isset3, "ARG", ARG_STRING, &stringarg1));
        BUG_ON (obj.addOption (true, 'd', "argd", "optional option with args", &isset4, "ARG", ARG_STRING, &stringarg2));

        BUG_ON (obj.parse (&index));
        BUG_ON (isset1 == 1);
        BUG_ON (isset2 == 1);
        BUG_ON (isset3 == 1);
        BUG_ON (isset4 == 1);
        BUG_ON (intarg1 == 10);
        BUG_ON (intarg2 == 10);
        BUG_ON (stringarg1);
        BUG_ON (!strcmp ("CCC", stringarg1));
        BUG_ON (stringarg2);
        BUG_ON (!strcmp ("DDD", stringarg2));
        BUG_ON (index == 7);
    }
    {
        char* argv[] = {"unittest14", "--arga=0xa", "--argb", "10", "--argc=CCC", "--argd", "DDD", "ABCD", "EFGH"};
        int argc = 9;
        isset1 = isset2 = isset3 = isset4 = -1;
        stringarg1 = stringarg2 = NULL;
        intarg1 = intarg2 = -1;
        int index = 0;

        cCmdline obj(argc, (char**)argv);

        BUG_ON (obj.addOption (true, 'a', "arga", "optional option with args", &isset1, "ARG", ARG_INT, &intarg1));
        BUG_ON (obj.addOption (true, 'b', "argb", "optional option with args", &isset2, "ARG", ARG_INT, &intarg2));
        BUG_ON (obj.addOption (true, 0, "argc", "optional option with args", &isset3, "ARG", ARG_STRING, &stringarg1));
        BUG_ON (obj.addOption (true, 'd', "argd", "optional option with args", &isset4, "ARG", ARG_STRING, &stringarg2));

        BUG_ON (obj.parse (&index));
        BUG_ON (isset1 == 1);
        BUG_ON (isset2 == 1);
        BUG_ON (isset3 == 1);
        BUG_ON (isset4 == 1);
        BUG_ON (intarg1 == 10);
        BUG_ON (intarg2 == 10);
        BUG_ON (stringarg1);
        BUG_ON (!strcmp ("CCC", stringarg1));
        BUG_ON (stringarg2);
        BUG_ON (!strcmp ("DDD", stringarg2));
        BUG_ON (index == 7);
    }
    {
        char* argv[] = {"unittest15", "-vvv"};
        int argc = 2;
        isset1 = isset2 = isset3 = isset4 = -1;
        int index = 0;

        cCmdline obj(argc, (char**)argv);

        BUG_ON (obj.addOption (true, 'v', "argd", "optional option without args", &isset1));

        BUG_ON (obj.parse (&index));
        BUG_ON (isset1 == 3);
    }
    {
        char* argv[] = {"unittest16", "-vv", "--arga"};
        int argc = 3;
        isset1 = isset2 = isset3 = isset4 = -1;
        int index = 0;

        cCmdline obj(argc, (char**)argv);

        BUG_ON (obj.addOption (true, 'v', nullptr, "optional short-only option without args", &isset1));
        BUG_ON (obj.addOption (true, 0, "arga", "optional long-only option without args", &isset2));

        BUG_ON (obj.parse (&index));
        BUG_ON (isset1 == 2);
        BUG_ON (isset2 == 1);
    }
    {
        char* argv[] = {"unittest17", "-a"};
        int argc = 2;
        intarg1 = intarg2 = -2;
        isset1 = isset2 = isset3 = isset4 = -1;
        int index = 0;

        cCmdline obj(argc, (char**)argv);

        BUG_ON (obj.addOption (true, 'a', "arga", "optional option with args", &isset1, "ARG", ARG_INT, &intarg1));

        BUG_ON (!obj.parse (&index));
        BUG_ON (isset1 == 0);
        BUG_ON (intarg1 == -2);
    }
    {
        char* argv[] = {"unittest18", "-a"};
        int argc = 2;
        intarg1 = intarg2 = -2;
        isset1 = isset2 = isset3 = isset4 = -1;
        int index = 0;

        cCmdline obj(argc, (char**)argv);

        BUG_ON (obj.addOption (true, 'a', nullptr, "optional short-only option with args", &isset1, "ARG", ARG_INT, &intarg1));

        BUG_ON (!obj.parse (&index));
        BUG_ON (isset1 == 0);
        BUG_ON (intarg1 == -2);
    }
    {
        char* argv[] = {"unittest19", "--arga"};
        int argc = 2;
        intarg1 = intarg2 = -2;
        isset1 = isset2 = isset3 = isset4 = -1;
        int index = 0;

        cCmdline obj(argc, (char**)argv);

        BUG_ON (obj.addOption (true, 0, "arga", "optional long-only option with args", &isset1, "ARG", ARG_INT, &intarg1));

        BUG_ON (!obj.parse (&index));
        BUG_ON (isset1 == 0);
        BUG_ON (intarg1 == -2);
    }
    {
        char* argv[] = {"unittest20", "-a", "-b"};
        int argc = 3;
        intarg1 = intarg2 = 0;
        isset1 = isset2 = isset3 = isset4 = -1;
        int index = 0;

        cCmdline obj(argc, (char**)argv);

        BUG_ON (obj.addOption (true, 'a', "arga", "optional option without args", &isset1));

        BUG_ON (!obj.parse (&index));
        BUG_ON (isset1 == 1);
    }
    {
        char* argv[] = {"unittest21", "-a", "--berta"};
        int argc = 3;
        intarg1 = intarg2 = 0;
        isset1 = isset2 = isset3 = isset4 = -1;
        int index = 0;

        cCmdline obj(argc, (char**)argv);

        BUG_ON (obj.addOption (true, 'a', "arga", "optional option without args", &isset1));

        BUG_ON (!obj.parse (&index));
        BUG_ON (isset1 == 1);
    }
    {
        char* argv[] = {"unittest23", "--arga", "2"};
        int argc = 3;
        intarg1 = intarg2 = -2;
        isset1 = isset2 = isset3 = isset4 = -1;
        int index = 0;

        cCmdline obj(argc, (char**)argv);

        BUG_ON (obj.addOption (true, 0, "arga", "optional long-only option optional with args", &isset1, "ARG", ARG_INT, &intarg1, true));

        BUG_ON (obj.parse (&index));
        BUG_ON (isset1 == 1);
        BUG_ON (intarg1 == -2);
    }
    {
        char* argv[] = {"unittest23", "--arga=2"};
        int argc = 2;
        intarg1 = intarg2 = -2;
        isset1 = isset2 = isset3 = isset4 = -1;
        int index = 0;

        cCmdline obj(argc, (char**)argv);

        BUG_ON (obj.addOption (true, 0, "arga", "optional long-only option optional with args", &isset1, "ARG", ARG_INT, &intarg1, true));

        BUG_ON (obj.parse (&index));
        BUG_ON (isset1 == 1);
        BUG_ON (intarg1 == 2);
    }
}
#endif

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
            this->help = 0;
            this->version = 0;

            cmdline.addOption  (true, 'h', "help", "Display this text", &help, nullptr, ARG_NO, nullptr, false, true);
            cmdline.addOption  (true, 0, "version", "Show detailed version infos", &version, nullptr, ARG_NO, nullptr, false, true);
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
        if (version)
        {
            printVersion ();
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
        version = " V" APP_VERSION;
        nn::Console::Print ("\n%s%s - %s\n\nUsage: %s\n\nOptions:\n", name, version, brief, usage);
        cmdline.printOptions ();
        nn::Console::Print ("\n%s\n\n", description);
    }
    void printVersion ()
    {
        const char* version = APP_VERSION;
        nn::Console::Print ("%s version %s (%s)\n", name, version, BUILD_TIME);
#ifdef GIT_COMMIT
        nn::Console::Print ("build: %s-%s (%s)\n", GIT_BRANCH, GIT_COMMIT, BUILD_TYPE);
#endif
    }

    // adds (optional) integer option with argument
    bool addCmdLineOption (bool optional, char shortname, const char* longname, const char* argname, const char* description,
            int* arg)
    {
        return cmdline.addOption (optional, shortname, longname, description, nullptr, argname, ARG_INT, (void*)arg, false);
    }
    // adds (optional) integer long-only-option with optional argument
    bool addCmdLineOption (bool optional, const char* longname, const char* argname, const char* description,
            int* optSet, int* arg)
    {
        return cmdline.addOption (optional, 0, longname, description, optSet, argname, ARG_INT, (void*)arg, true);
    }
    // adds (optional) string option with argument
    bool addCmdLineOption (bool optional, char shortname, const char* longname, const char* argname, const char* description,
            const char** arg)
    {
        return cmdline.addOption (optional, shortname, longname, description, nullptr, argname, ARG_STRING, (void*)arg, false);
    }
    // adds (optional) string long-only-option with optional argument
    bool addCmdLineOption (bool optional, const char* longname, const char* argname, const char* description,
            int* optSet, const char** arg)
    {
        return cmdline.addOption (optional, 0, longname, description, optSet, argname, ARG_STRING, (void*)arg, true);
    }
    // adds boolean (optional) option without argument
    bool addCmdLineOption (bool optional, char shortname, const char* longname, const char* description, int* optSet)
    {
        return cmdline.addOption (optional, shortname, longname, description, optSet);
    }

private:
    const char* name;
    const char* brief;
    const char* usage;
    const char* description;
    int help;
    int version;
    cCmdline cmdline;
};

#endif /* CMDLINE_HPP_ */

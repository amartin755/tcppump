// SPDX-License-Identifier: GPL-3.0-only
/*
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2021 Andreas Martin (netnag@mailbox.org)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include <cstdarg>
#include <cstdio>

#include "console.hpp"
#include "bug.hpp"

Console::out_level Console::level = Normal;
#ifdef MT_CONSOLE
    std::mutex Console::mtx;
#endif


void Console::SetPrintLevel (out_level lvl)
{
    BUG_ON ((lvl < Silent) || (lvl > Debug));
    level = lvl;
}

bool Console::PrintError (const char* format, ...)
{
    bool ret;
    va_list args;
    va_start (args, format);

    ret = Console::print (Console::Error, format, args);

    va_end (args);
    return ret;
}
bool Console::Print (const char* format, ...)
{
    bool ret;
    va_list args;
    va_start (args, format);
#ifdef MT_CONSOLE
    mtx.lock();
#endif
    ret = Console::print (Console::Normal, format, args);
#ifdef MT_CONSOLE
    mtx.unlock();
#endif

    va_end (args);
    return ret;
}
bool Console::PrintVerbose (const char* format, ...)
{
    bool ret;
    va_list args;
    va_start (args, format);

    ret = Console::print (Console::Verbose, format, args);

    va_end (args);
    return ret;
}
bool Console::PrintMoreVerbose (const char* format, ...)
{
    bool ret;
    va_list args;
    va_start (args, format);

    ret = Console::print (Console::MoreVerbose, format, args);

    va_end (args);
    return ret;
}
bool Console::PrintMostVerbose (const char* format, ...)
{
    bool ret;
    va_list args;
    va_start (args, format);

    ret = Console::print (Console::MostVerbose, format, args);

    va_end (args);
    return ret;
}
bool Console::PrintDebug (const char* format, ...)
{
    bool ret;
    va_list args;
    va_start (args, format);

    ret = Console::print (Console::Debug, format, args);

    va_end (args);
    return ret;
}

bool Console::print (out_level lvl, const char* format, va_list ap)
{
    if (lvl > level)
        return false;

     // we always print to stderr to be able to separate piped in/output from console prints
    bool ret = vfprintf (stderr, format, ap) >= 0;
    fflush (stderr);
    return ret;
}

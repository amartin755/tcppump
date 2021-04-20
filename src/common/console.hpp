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


#ifndef CONSOLE_HPP_
#define CONSOLE_HPP_

#include <cstdarg>


class Console
{
public:
    static bool PrintError (const char* format, ...);
    static bool Print (const char* format, ...);
    static bool PrintVerbose (const char* format, ...);
    static bool PrintMoreVerbose (const char* format, ...);
    static bool PrintMostVerbose (const char* format, ...);
    static bool PrintDebug (const char* format, ...);

    enum out_level {Silent = 1, Error = 2, Normal = 3, Verbose = 4, MoreVerbose = 5, MostVerbose = 6, Debug = 7};
    static void SetPrintLevel (out_level lvl);

private:
    static bool print (out_level lvl, const char* format, va_list ap);

private:
    static out_level level;
};

#endif /* CONSOLE_HPP_ */

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


#include <cstdio>
#include <termios.h>
#include <unistd.h>

namespace tcppump
{
    // emulates the behavior of DOS' getch, which returns unbuffered STDIN
    int getch (void)
    {
        int c;
        struct termios oldt, newt;

        // save current terminal status
        tcgetattr (STDIN_FILENO, &oldt);

        // set terminal to non-canonical mode
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr (STDIN_FILENO, TCSANOW, &newt);
        c = getchar();

        // restore terminal status
        tcsetattr (STDIN_FILENO, TCSANOW, &oldt);

        return c;
    }
}


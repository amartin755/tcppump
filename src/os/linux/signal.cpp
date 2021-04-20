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


#include <csignal>

#include "signal.hpp"

static volatile std::sig_atomic_t gSigIntStatus;
static void sigintHandler (int signal)
{
    if (signal == SIGINT)
    {
        gSigIntStatus++;
    }
}

void cSignal::sigintEnable (void)
{
     std::signal (SIGINT, sigintHandler);
}

bool cSignal::sigintSignalled (void)
{
    return !!gSigIntStatus;
}


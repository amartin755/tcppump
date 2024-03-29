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


#include <windows.h>
#include <cstdlib>

#include "bug.hpp"
#include "signal.hpp"
#include "console.hpp"

/* FIXME This code is only a workaround and it does not work in MSYS2 shells
 *
 * On windows a thread is not able to stop sleeping when a signal has triggered.
 * Thus special handling is needed to gracefully shutdown/interrupt packet transmission.
 * TODO On CTRL_C_EVENT send an event object (CreateEvent, SetEvent & Co). All WaitForSingleObject calls must be replaced by
 * WaitForMultipleObjects and must also listen to this event object. If the event has occurred we can gracefully stop packet
 * transmission jobs.
 */
static HANDLE sigintEvent = INVALID_HANDLE_VALUE;
static volatile int gSigIntStatus;
static std::function<void(void)> callback = nullptr;

static BOOL signalHandler(DWORD dwType)
{
    switch (dwType)
    {
    case CTRL_C_EVENT:
        gSigIntStatus++;
        SetEvent (sigintEvent);
        if (gSigIntStatus == 1)
            Console::PrintError ("Waiting for packet transmission job to finish\n");
        if (callback != nullptr)
            callback();
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

void cSignal::sigintEnable (void)
{
    ::SetConsoleCtrlHandler((PHANDLER_ROUTINE)signalHandler, TRUE);
}

bool cSignal::sigintSignalled (void)
{
    return !!gSigIntStatus;
}

HANDLE cSignal::sigintGetEventHandle (void)
{
    if (sigintEvent != INVALID_HANDLE_VALUE)
        return sigintEvent;

    sigintEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
    BUG_ON (!sigintEvent);
    return sigintEvent;
}

void cSignal::sigintSetCallback (std::function<void(void)> &func)
{
    callback = func;    
}

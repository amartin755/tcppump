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


#include <ctime>
#include <chrono>
#include <thread>

#include "timeval.hpp"
#include "sleep.hpp"
#include "bug.hpp"

#ifdef WITH_UNITTESTS
#include "console.hpp"
#endif

/*
 * NOTE: The following is 100% C++11 and therefore portable. In my tests on Windows
 * the code was not really working, because sleep_for returns immediately when nanoseconds
 * are used, which makes it impossible to measure the sleep resolution. In addition
 * high_resolution_clock::now() seems to suspend the caller ~15ms. This makes it impossible
 * to use it for busy-waiting loops.
 *
 * Tested on Win10 (64bit) gcc (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0
 */

using namespace std;

namespace tcppump
{

static cTimeval resolution;


static void busyWaitUs (uint64_t us)
{
    if (!us)
        return;

    auto t1 = chrono::high_resolution_clock::now();
    auto t2 = t1;

    do
    {
        t2 = chrono::high_resolution_clock::now();
    }
    while ((uint64_t)(chrono::duration_cast<chrono::microseconds>(t2 - t1).count()) < us);
}

cTimeval SleepInit ()
{
    const int LOOPS = 1000;
    double totalTime = 0;

    // measure the resolution of sleep_for function
    for (int n = 0; n < LOOPS; n++)
    {
        auto t1 = chrono::high_resolution_clock::now();
        this_thread::sleep_for(chrono::nanoseconds(1));
        auto t2 = chrono::high_resolution_clock::now();

        auto elapsedTime = chrono::duration_cast<chrono::nanoseconds>(t2 - t1);
        totalTime += elapsedTime.count ();
    }
    totalTime /= 1000;
    resolution.setUs (totalTime / LOOPS);

    return resolution;
}

void Sleep (const cTimeval& t)
{
    BUG_ON (resolution.isNull());

    if (t < resolution)
    {
        busyWaitUs (t.us ());
    }
    else
    {
        cTimeval tRounded(t);
        tRounded.roundDown(resolution);

        auto t1 = chrono::high_resolution_clock::now();
        this_thread::sleep_for(chrono::microseconds(tRounded.us()));
        auto t2 = chrono::high_resolution_clock::now();
        auto elapsedUs = chrono::duration_cast<chrono::microseconds>(t2 - t1);

        if (t.us () > (uint64_t)elapsedUs.count())
            busyWaitUs (t.us () - elapsedUs.count());
    }

}

#ifdef WITH_UNITTESTS



static double measure (uint64_t us)
{
    double deviation;
    cTimeval t;
    t.setUs(us);

    auto t1 = chrono::high_resolution_clock::now();
    Sleep (t);
    auto t2 = chrono::high_resolution_clock::now();
    auto elapsedUs = chrono::duration_cast<chrono::microseconds>(t2 - t1);

    deviation = (((double)elapsedUs.count() - (double)us) / (double)us) * 100.0;

    ::Console::PrintDebug ("measure %u = %.3f usec (error = %.1f%%)\n", (unsigned) us, (double)elapsedUs.count(), deviation);

    return deviation;
}

void SleepUnitTest ()
{
    measure (0);
    measure (1);
    measure (10);
    measure (50);
    measure (60);
    measure (70);
    measure (100);
    measure (200);
    measure (1000);
    measure (10000);
    measure (12345);
    measure (20000);
}

#endif
}





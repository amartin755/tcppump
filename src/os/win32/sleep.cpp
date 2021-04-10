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


#include <windows.h>

#include "bug.hpp"

#include "timeval.hpp"
#ifdef WITH_UNITTESTS
#include "console.hpp"
#endif

namespace tcppump
{

static unsigned ticksPerUs;
static cTimeval resolution;

static void busyWaitUs (uint64_t us)
{
    if (!us)
        return;

    LARGE_INTEGER t1, t2;
    uint64_t ticks = us * ticksPerUs;

    ::QueryPerformanceCounter (&t1);

    do
    {
        ::QueryPerformanceCounter (&t2);
    }
    while ((uint64_t)(t2.QuadPart - t1.QuadPart) < ticks);
}

cTimeval SleepInit ()
{
    LARGE_INTEGER t1, t2, freq;
    double elapsedTime = 0;
    const int LOOPS = 50;

    ::QueryPerformanceFrequency (&freq);
    ticksPerUs = unsigned(freq.QuadPart / 1000000.0);

    // mearure the resolution of Windows' Sleep function
    for (int n = 0; n < LOOPS; n++)
    {
        ::QueryPerformanceCounter (&t1);
        ::Sleep(1);
        ::QueryPerformanceCounter (&t2);

        elapsedTime += (double)(t2.QuadPart-t1.QuadPart) / ticksPerUs;
    }
    resolution.setUs (uint64_t(elapsedTime / LOOPS));
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

        LARGE_INTEGER t1, t2;
        ::QueryPerformanceCounter (&t1);
        ::Sleep ((DWORD)t.ms());
        ::QueryPerformanceCounter (&t2);

        uint64_t elapsedUs = (uint64_t)(t2.QuadPart - t1.QuadPart) / ticksPerUs;
        if (t.us () > elapsedUs)
            busyWaitUs (t.us () - elapsedUs);
    }
}

#ifdef WITH_UNITTESTS



static double measure (uint64_t us)
{
    LARGE_INTEGER t1, t2;
    double deviation;
    cTimeval t;
    t.setUs(us);

    ::QueryPerformanceCounter (&t1);
    Sleep (t);
    ::QueryPerformanceCounter (&t2);
    double elapsedUs = (double)(t2.QuadPart-t1.QuadPart) / ticksPerUs;

    deviation = ((elapsedUs - (double)us) / (double)us) * 100.0;

    ::Console::PrintDebug ("measure %u = %.3f usec (error = %.1f%%)\n", (unsigned) us, (double)elapsedUs, deviation);

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





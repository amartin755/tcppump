/**
 * tcppump <https://github.com/amartin755/netnag>
 * Copyright (C) 2012-2019 Andreas Martin (netnag@mailbox.org)
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
#include <cassert>

#include "timeval.hpp"

namespace tcppump
{

static unsigned ticksPerUs;
static cTimeval accuracy;

static void busyWaitUs (uint64_t us)
{
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

	// mearure the accuracy of Windows' Sleep function
	for (int n = 0; n < LOOPS; n++)
	{
		::QueryPerformanceCounter (&t1);
		::Sleep(1);
		::QueryPerformanceCounter (&t2);

		elapsedTime += (double)(t2.QuadPart-t1.QuadPart) / ticksPerUs;
	}
	accuracy.setUs (elapsedTime / LOOPS);
	return accuracy;
}

void Sleep (const cTimeval& t)
{
	assert (!accuracy.isNull());

	if (t < accuracy)
	{
		busyWaitUs (t.us ());
	}
	else
	{
		LARGE_INTEGER t1, t2;
		::QueryPerformanceCounter (&t1);
		::Sleep ((DWORD)t.ms());
		::QueryPerformanceCounter (&t2);

		uint64_t elapsedUs = (uint64_t)(t2.QuadPart - t1.QuadPart) / ticksPerUs;
		busyWaitUs (t.us () - elapsedUs);
	}
}

}





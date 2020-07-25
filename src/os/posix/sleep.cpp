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

#include <cassert>
#include <ctime>
#include <chrono>
#include <limits>

#include "timeval.hpp"

namespace tcppump
{

static cTimeval resolution;


static void busyWaitUs (uint64_t us)
{
	auto t1 = std::chrono::high_resolution_clock::now();
	auto t2 = t1;

	do
	{
		t2 = std::chrono::high_resolution_clock::now();
	}
	while ((uint64_t)(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) < us);
}

cTimeval SleepInit ()
{
	const int LOOPS = 1000;
	struct timespec clockres;
	double totalTime = 0;


	if (::clock_getres (CLOCK_MONOTONIC, &clockres))
	{
		// fallback, in case monotonic clock is not supported
		clockres.tv_sec  = 0;
		clockres.tv_nsec = 1000;
	}

	// measure the resolution of nanosleep function
	for (int n = 0; n < LOOPS; n++)
	{
		auto t1 = std::chrono::high_resolution_clock::now();
		if (::nanosleep(&clockres, NULL))
			continue;
		auto t2 = std::chrono::high_resolution_clock::now();

		auto elapsedTime = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);
		totalTime += elapsedTime.count ();
	}
	totalTime /= 1000;
	resolution.setUs (totalTime / LOOPS);

	return resolution;
}

void Sleep (const cTimeval& t)
{
	assert (!resolution.isNull());

	if (t < resolution)
	{
		busyWaitUs (t.us ());
	}
	else
	{
		cTimeval tRounded(t);
		tRounded.roundDown(resolution);

		struct timespec sleeptime;
		sleeptime.tv_sec = tRounded.s();
		sleeptime.tv_nsec = tRounded.us()*1000;

		auto t1 = std::chrono::high_resolution_clock::now();
		::nanosleep (&sleeptime, NULL);
		auto t2 = std::chrono::high_resolution_clock::now();
		auto elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);

		if (t.us () > (uint64_t)elapsedUs.count())
			busyWaitUs (t.us () - elapsedUs.count());
	}

}


}





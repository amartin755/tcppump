/**
 * NETNAG <https://github.com/amartin755/netnag>
 * Copyright (C) 2012-2016 Andreas Martin (netnag@mailbox.org)
 *
 * timeval.cpp
 *
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


#include "timeval.hpp"

cTimeval::cTimeval ()
{
	value.tv_sec  = 0;
	value.tv_usec = 0;
}

cTimeval::cTimeval (struct timeval *t)
{
	set (t);
}

cTimeval::cTimeval (long long usTime)
{
	set (usTime);
}

cTimeval::~cTimeval ()
{
}

const struct timeval* cTimeval::get ()
{
	return &value;
}

cTimeval::operator long long()
{
	return value.tv_sec * 1000000LL + (long long)value.tv_usec;
}

void cTimeval::set (struct timeval *tv)
{
	value = *tv;
}

void cTimeval::set (long long usTime)
{
	value.tv_sec  = (long)(usTime / 1000000LL);
	value.tv_usec = (long)(usTime % 1000000LL);
}

void cTimeval::set (cTimeval& t)
{
	value = *t.get ();
}

const struct timeval* cTimeval::add (const struct timeval *t)
{
	add (&value, &value, t);
	return &value;
}

void cTimeval::add (cTimeval& t)
{
	add (t.get());
}

const struct timeval* cTimeval::sub (const struct timeval *t)
{
	sub (&value, &value, t);
	return &value;
}

int cTimeval::sub (struct timeval *result, const struct timeval *x, const struct timeval *y)
{
	struct timeval t = *y;

	/* Perform the carry for the later subtraction by updating y. */
	if (x->tv_usec < y->tv_usec)
	{
		int nsec = (t.tv_usec - x->tv_usec) / 1000000 + 1;
		t.tv_usec -= 1000000 * nsec;
		t.tv_sec += nsec;
	}
	if (x->tv_usec - y->tv_usec > 1000000)
	{
		int nsec = (x->tv_usec - t.tv_usec) / 1000000;
		t.tv_usec += 1000000 * nsec;
		t.tv_sec -= nsec;
	}

	/* Compute the time remaining to wait.
	 tv_usec is certainly positive. */
	result->tv_sec = x->tv_sec - t.tv_sec;
	result->tv_usec = x->tv_usec - t.tv_usec;

	/* Return 1 if result is negative. */
	return x->tv_sec < t.tv_sec;
}

void cTimeval::add (struct timeval *result, const struct timeval *time1, const struct timeval *time2)
{
	result->tv_sec  = time1->tv_sec  + time2->tv_sec;
	result->tv_usec = time1->tv_usec + time2->tv_usec;
	if (result->tv_usec >= 1000000L)
	{ /* Carry? */
		result->tv_sec++;
		result->tv_usec = result->tv_usec - 1000000L;
	}

}

#ifdef WITH_UNITTESTS
void cTimeval::unitTest ()
{

}
#endif

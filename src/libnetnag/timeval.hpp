/**
 * NETNAG <https://github.com/amartin755/netnag>
 * Copyright (C) 2012-2016 Andreas Martin (netnag@mailbox.org)
 *
 * timeval.hpp
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


#ifndef TIMEVAL_HPP_
#define TIMEVAL_HPP_

#include "timeval.h" // struct timeval

class cTimeval
{
public:
	cTimeval ();
	cTimeval (struct timeval *);
	cTimeval (long long);
	~cTimeval ();
	const struct timeval* get ();
	operator long long ();
	void set (struct timeval *);
	void set (long long);
	void set (cTimeval&);
	const struct timeval* add (const struct timeval *);
	void add (cTimeval&);
	const struct timeval* sub (const struct timeval *);
	static int sub(struct timeval *result, const struct timeval *x,
			const struct timeval *y);
	static void add(struct timeval *result, const struct timeval *time1,
			const struct timeval *time2);

#ifdef WITH_UNITTESTS
	static void unitTest ();
#endif

private:
	struct timeval value;
};

#endif /* TIMEVAL_HPP_ */

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

#include <cstdint>
#include "timeval.h" // struct timeval
#ifdef WITH_UNITTESTS
#include <cassert>
#endif

class cTimeval
{
public:
	cTimeval ()
	{
		value = 0;
	}
	cTimeval (const struct timeval& tv)
	{
		set (tv);
	}
	cTimeval (uint64_t seconds)
	{
		setS (seconds);
	}
	void clear ()
	{
		value = 0;
	}
	bool isNull () const
	{
		return value == 0;
	}
	struct timeval timeval() const
	{
		struct timeval tv;
		tv.tv_sec  = decltype(tv.tv_sec)(this->s());
		tv.tv_usec = decltype(tv.tv_usec)(this->us() % 1000000ULL);

		return tv;
	}
	uint64_t ns() const
	{
		return value * 1000;
	}
	uint64_t us() const
	{
		return value;
	}
	void setUs (uint64_t us)
	{
		value = us;
	}
	uint64_t ms() const
	{
		return value / 1000;
	}
	void setMs (uint64_t ms)
	{
		value = ms * 1000;
	}
	uint64_t s() const
	{
		return value / 1000000;
	}
	void setS (uint64_t s)
	{
		value = s * 1000000;
	}
	void set (const cTimeval& val)
	{
		this->value = val.value;
	}
	void set (const struct timeval &tv)
	{
		value = (uint64_t)(tv.tv_sec * 1000000) + (uint64_t)tv.tv_usec;
	}
	cTimeval& add (const cTimeval& val)
	{
		value += val.value;
		return *this;
	}
	cTimeval& add (const struct timeval &tv)
	{
		add (cTimeval (tv));
		return *this;
	}
	cTimeval& sub (const cTimeval& val)
	{
		value -= val.value;
		return *this;
	}
	cTimeval& sub (const struct timeval &tv)
	{
		sub (cTimeval (tv));
		return *this;
	}
	cTimeval& roundDown (const cTimeval& div)
	{
		value = (value / div.value) * div.value;
		return *this;
	}
	bool operator== (const cTimeval &val) const
	{
		return value == val.value;
	}
	bool operator!= (const cTimeval &val) const
	{
		return value != val.value;
	}
	bool operator< (const cTimeval &val) const
	{
		return value < val.value;
	}
	bool operator> (const cTimeval &val) const
	{
		return value > val.value;
	}

private:
	uint64_t value;


#ifdef WITH_UNITTESTS
public:
	static void unitTest ()
	{
		struct timeval tv1;
		tv1.tv_sec = 0;
		tv1.tv_usec = 999999;
		struct timeval tv2;
		tv2.tv_sec = 0;
		tv2.tv_usec = 1;

		assert (cTimeval (tv1).s() == 0);
		assert (cTimeval (tv2).s() == 0);
		assert (cTimeval (tv1).ms() == 999);
		assert (cTimeval (tv2).ms() == 0);
		assert (cTimeval (tv1).us() == 999999);
		assert (cTimeval (tv2).us() == 1);

		cTimeval v;
		assert (v.s() == 0);
		assert (v.ms() == 0);
		assert (v.us() == 0);
		v.set (tv1);
		assert (v.s() == 0);
		assert (v.ms() == 999);
		assert (v.us() == 999999);
		v.add(tv2);
		assert (v.s() == 1);
		assert (v.ms() == 1000);
		assert (v.us() == 1000000);
		v.add(v).sub(tv2);
		assert (v.s() == 1);
		assert (v.ms() == 1999);
		assert (v.us() == 1999999);
		v.sub(tv2);
		struct timeval tv3 = v.timeval();
		assert (tv3.tv_sec == 1);
		assert (tv3.tv_usec == 999998);
		v.sub(v);
		assert (v.s() == 0);
		assert (v.ms() == 0);
		assert (v.us() == 0);
		struct timeval tv4 = v.timeval();
		assert (tv4.tv_sec == 0);
		assert (tv4.tv_usec == 0);

		assert (cTimeval() == v);
		assert (!(cTimeval() != v));
		assert (!(cTimeval(tv1) == cTimeval(tv2)));
		assert (cTimeval(tv1) != cTimeval(tv2));

		assert (cTimeval(2) < cTimeval(3));
		assert (cTimeval(1) > cTimeval(0));
	}
#endif
};

#endif /* TIMEVAL_HPP_ */

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


#ifndef TIMEVAL_HPP_
#define TIMEVAL_HPP_

#include <cstdint>
#include <chrono>

#include "timeval.h" // struct timeval
#ifdef WITH_UNITTESTS
#include "bug.hpp"
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
    cTimeval& now ()
    {
        auto currTime = std::chrono::system_clock::now();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(currTime.time_since_epoch());
        value = us.count();
        return *this;
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
    cTimeval& mul (double val)
    {
        value = (uint64_t)((double)value * val);
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

        BUG_IF_NOT (cTimeval (tv1).s() == 0);
        BUG_IF_NOT (cTimeval (tv2).s() == 0);
        BUG_IF_NOT (cTimeval (tv1).ms() == 999);
        BUG_IF_NOT (cTimeval (tv2).ms() == 0);
        BUG_IF_NOT (cTimeval (tv1).us() == 999999);
        BUG_IF_NOT (cTimeval (tv2).us() == 1);

        cTimeval v;
        BUG_IF_NOT (v.s() == 0);
        BUG_IF_NOT (v.ms() == 0);
        BUG_IF_NOT (v.us() == 0);
        v.set (tv1);
        BUG_IF_NOT (v.s() == 0);
        BUG_IF_NOT (v.ms() == 999);
        BUG_IF_NOT (v.us() == 999999);
        v.add(tv2);
        BUG_IF_NOT (v.s() == 1);
        BUG_IF_NOT (v.ms() == 1000);
        BUG_IF_NOT (v.us() == 1000000);
        v.add(v).sub(tv2);
        BUG_IF_NOT (v.s() == 1);
        BUG_IF_NOT (v.ms() == 1999);
        BUG_IF_NOT (v.us() == 1999999);
        v.sub(tv2);
        struct timeval tv3 = v.timeval();
        BUG_IF_NOT (tv3.tv_sec == 1);
        BUG_IF_NOT (tv3.tv_usec == 999998);
        v.sub(v);
        BUG_IF_NOT (v.s() == 0);
        BUG_IF_NOT (v.ms() == 0);
        BUG_IF_NOT (v.us() == 0);
        struct timeval tv4 = v.timeval();
        BUG_IF_NOT (tv4.tv_sec == 0);
        BUG_IF_NOT (tv4.tv_usec == 0);

        BUG_IF_NOT (cTimeval() == v);
        BUG_IF_NOT (!(cTimeval() != v));
        BUG_IF_NOT (!(cTimeval(tv1) == cTimeval(tv2)));
        BUG_IF_NOT (cTimeval(tv1) != cTimeval(tv2));

        BUG_IF_NOT (cTimeval(2) < cTimeval(3));
        BUG_IF_NOT (cTimeval(1) > cTimeval(0));
        BUG_IF_NOT (cTimeval(10).mul(1.5) == cTimeval(15));
    }
#endif
};

#endif /* TIMEVAL_HPP_ */

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


#ifndef CONTROLFLOW_HPP_
#define CONTROLFLOW_HPP_

#include "linkable.hpp"

class cLoop : public cLinkable
{
public:
    cLoop (int counter) : m_runs (counter), m_counter (counter), m_end (nullptr)
    {
    }

    ~cLoop ()
    {
    }

    void setEnd (cLinkable* end)
    {
        m_end = end;
    }

    cLinkable* getNext (void)
    {
        if (m_end && (m_counter-- <= 0))
        {
            m_counter = m_runs;
            return m_end->getNext ();
        }
        else
        {
            return cLinkable::getNext ();
        }
    }

private:
    int m_runs;
    int m_counter;    // loop counter, if counter <= 0 we jump to m_end
    cLinkable* m_end; // next instruction after the loop block
};

class cGoto : public cLinkable
{
public:
    cGoto (cLinkable* to) : m_jumpTo (to)
    {
    }

    ~cGoto ()
    {
    }

    cLinkable* getNext (void)
    {
        return m_jumpTo;
    }

private:
    cLinkable* m_jumpTo;
};

#endif /* CONTROLFLOW_HPP_ */

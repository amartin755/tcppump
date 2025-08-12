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


#ifndef LINKABLE_HPP_
#define LINKABLE_HPP_

#include "timeval.hpp"

class cLinkable
{
public:
    cLinkable ()
    {
        m_next = nullptr;
    }
    virtual ~cLinkable ()
    {
    }
    inline cLinkable* getNext (void)
    {
        return m_next;
    }
    inline void setNext (cLinkable* next)
    {
        m_next = next;
    }
    inline const cTimeval& getTime (void)
    {
        return m_t;
    }
    inline void setTime (const cTimeval& t)
    {
        m_t = t;
    }

private:
    cLinkable *m_next;
    cTimeval m_t;
};

#endif /* LINKABLE_HPP_ */

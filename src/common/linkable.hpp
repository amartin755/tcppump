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
        next = nullptr;
        lineNumber = 0;
    }
    virtual ~cLinkable ()
    {
    }
    inline cLinkable* getNext (void)
    {
        return next;
    }
    inline void setNext (cLinkable* next)
    {
        this->next = next;
    }
    inline const cTimeval& getTime (void)
    {
        return t;
    }
    inline void setTime (const cTimeval& t)
    {
        this->t = t;
    }
    inline void setLineNumber (int line)
    {
        lineNumber = line;
    }
    inline int getLineNumber (void)
    {
        return lineNumber;
    }

private:
    cLinkable *next;
    cTimeval t;
    int lineNumber;
};

#endif /* LINKABLE_HPP_ */

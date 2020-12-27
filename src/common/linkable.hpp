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

#ifndef LINKABLE_HPP_
#define LINKABLE_HPP_

#include "timeval.hpp"

class cLinkable
{
public:
    cLinkable ()
    {
        next = nullptr;
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

private:
    cLinkable *next;
    cTimeval t;
};

#endif /* LINKABLE_HPP_ */

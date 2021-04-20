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


#ifndef FORMATEXCEPTION_HPP_
#define FORMATEXCEPTION_HPP_

const int exParUnknown = 1;
const int exParRange   = 2;
const int exParFormat  = 3;
const int exParUnused  = 4;

class FormatException
{
public:
    FormatException (int cause, const char* val, int valLen = 0)
    {
        this->cause  = cause;
        this->val    = val;
        this->valLen = valLen;
    }

    int what ()
    {
        return cause;
    }
    const char* why ()
    {
        switch (cause)
        {
        case exParUnknown:
            return "Unknown or missing parameter";
        case exParRange:
            return "Range of parameter violated";
        case exParFormat:
            return "Invalid parameter format";
        case exParUnused:
            return "Unexpected parameter";
        }
        return "";
    }

    const char* value ()
    {
        return val;
    }

    int valueLength ()
    {
        return valLen;
    }

private:
    int cause;
    const char* val;
    int valLen;
};



#endif /* FORMATEXCEPTION_HPP_ */

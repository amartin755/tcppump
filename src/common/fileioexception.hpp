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


#ifndef FILEIOEXCEPTION_HPP_
#define FILEIOEXCEPTION_HPP_


class FileIOException
{
public:
    enum errorType
    {
        OPEN, READ, WRITE
    };

    FileIOException (errorType cause, const char* val)
    {
        this->cause  = cause;
        this->val    = val;
    }

    const char* what ()
    {
        switch (cause)
        {
        case OPEN:
            return "Could not open file";
        case READ:
            return "Could not read file";
        case WRITE:
            return "Could not write file";
        }
        return "";
    }

    const char* value ()
    {
        return val;
    }


private:
    errorType cause;
    const char* val;
};



#endif /* FILEIOEXCEPTION_HPP_ */

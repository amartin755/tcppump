// SPDX-License-Identifier: GPL-3.0-only
/*
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2026 Andreas Martin (netnag@mailbox.org)
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


#ifndef PARSEEXCEPTION_HPP_
#define PARSEEXCEPTION_HPP_

#include <string>

class ParseException
{
public:
    ParseException (const char* inst, const char* errMsg, const char* errBegin, int errLen = 0)
    : m_errMsg (errMsg), m_errorBegin (errBegin), m_inst (inst), m_details (), m_errorLen (errLen)
    {
    }
    ParseException (const char* inst, const char* errMsg, const char* details, const char* errBegin, int errLen)
    : m_errMsg (errMsg), m_errorBegin (errBegin), m_inst (inst), m_details (!details ? "" : details), m_errorLen (errLen)
    {
    }

    const char* errorMsg () const
    {
        return m_errMsg.c_str();
    }

    const char* instruction () const
    {
        return m_inst;
    }

    const char* errorBegin () const
    {
        return m_errorBegin;
    }

    int errorLen () const
    {
        return m_errorLen;
    }

    const char* details () const
    {
        return m_details.empty() ? nullptr : m_details.c_str();
    }



private:
    const std::string m_errMsg;
    const char* m_errorBegin;
    const char* m_inst;
    const std::string m_details;
    int m_errorLen;
};

#endif
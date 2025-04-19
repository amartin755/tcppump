// SPDX-License-Identifier: GPL-3.0-only
/*
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2025 Andreas Martin (netnag@mailbox.org)
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


#ifndef UUID_HPP
#define UUID_HPP

#include <cstdint>
#include <cstring>
#include <stdexcept>

#include "bug.hpp"

//UUID as defined in RFC9562

class cUUID
{
public:
    static cUUID fromMD5 (const uint8_t* md5)
    {
        return cUUID (md5, 3);
    }
    static cUUID fromSHA1 (const uint8_t* sha1)
    {
        return cUUID (sha1, 5);
    }
    static cUUID fromString (const std::string& str)
    {
        return cUUID (str.c_str());
    }
    static cUUID fromZero ()
    {
        cUUID uuid;
        std::memset (&uuid.m_uuid, 0, sizeof(uuid.m_uuid));
        return uuid;
    }
    int version () const
    {
        return (m_uuid.asArray[6] >> 4 & 0x0F);
    }

    const uint8_t* asArray () const
    {
        return m_uuid.asArray;
    }

    std::string asString () const
    {
        // TODO when we have std::format we don't need to copy anymore
        char uuid[16*2+4+1];
        snprintf (uuid, sizeof (uuid), 
            "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x", 
            m_uuid.asArray[0],
            m_uuid.asArray[1],
            m_uuid.asArray[2],
            m_uuid.asArray[3],
            m_uuid.asArray[4],
            m_uuid.asArray[5],
            m_uuid.asArray[6],
            m_uuid.asArray[7],
            m_uuid.asArray[8],
            m_uuid.asArray[9],
            m_uuid.asArray[10],
            m_uuid.asArray[11],
            m_uuid.asArray[12],
            m_uuid.asArray[13],
            m_uuid.asArray[14],
            m_uuid.asArray[15]
        );
        return std::string (uuid);
    }

    static size_t stringSize ()
    {
        return sizeof ("xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx")-1;
    }
    static size_t rasSize ()
    {
        return sizeof (cUUID::m_uuid);
    }

private:
    cUUID ()
    {
    }

    cUUID (const uint8_t* uuid, uint8_t version = 0)
    {
        std::memcpy (m_uuid.asArray, uuid, sizeof (m_uuid.asArray));
        if (version > 0 && version < 16)
        {
            // set version field
            m_uuid.asArray[6] = (m_uuid.asArray[6] & 0x0f) | version;
            // set variant field
            m_uuid.asArray[8] = (m_uuid.asArray[8] & 0x3f) | 0x80;
        }
    }
    cUUID (const char* str)
    {
        //xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
        int n;
        int index = 0;
        char nibbles[3] = {0,0,0};

        for (n = 0; str[n] && n < 36; n++)
        {
            if (str[n] == '-')
            {
                if (unlikely(n != 8 && n != 13 && n != 18 && n != 23))
                    throw std::out_of_range ("invalid uuid string");
                else
                    continue;
            }
            if (unlikely (!isxdigit (str[n])))
            {
                throw std::out_of_range ("invalid uuid string");
            }
            nibbles [index & 1] = str[n];
            if (index & 1)
            {
                m_uuid.asArray[index >> 1] = (uint8_t)strtoul (nibbles, nullptr, 16);
            }
            index++;
        }
        if (unlikely (n != 36))
            throw std::out_of_range ("invalid uuid string length");
    }

    union uuid
    {
        uint64_t asInt64[2];
        uint8_t asArray[16];

    }m_uuid;

#ifdef WITH_UNITTESTS
public:
    static void unitTest ()
    {
        std::string uuidString ("00112233-4455-6677-8899-aabbccddeeff");
        uint8_t uuidArray[] = {0,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
        cUUID uuid = cUUID::fromString (uuidString);

        BUG_ON (std::memcmp (uuid.asArray (), uuidArray, sizeof (uuidArray)));
        BUG_ON (uuid.asString() != uuidString);
    }
#endif

};
#endif
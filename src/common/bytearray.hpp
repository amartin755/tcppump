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


#ifndef BYTEARRAY_HPP
#define BYTEARRAY_HPP

#include <cstdint>
#include <cstring>
#include <stdexcept>

#include "bug.hpp"


class cByteArray
{
    static const size_t DEFAULT_CHUNK = 2048;
public:
    cByteArray(const cByteArray&) = delete;
    cByteArray(const cByteArray&&) = delete;
    cByteArray& operator= (const cByteArray&) = delete;
    cByteArray& operator= (const cByteArray&&) = delete;

    // dynamic array
    cByteArray (size_t initialLength = 0, size_t chunksize = cByteArray::DEFAULT_CHUNK) 
    : m_chunkSize (chunksize), m_data (nullptr), m_capacity (0), m_currPos (0), m_dynSize (true), m_extData (false)
    {
        if (initialLength)
        {
            resize (initialLength);
        }
    }

    // fixed size array either with memory provided by caller (buffer != 0) or managed internally
    cByteArray (void* buffer, size_t size)
    : m_chunkSize (size), m_data ((uint8_t*)buffer), m_capacity (size), m_currPos (0), m_dynSize (false), m_extData (buffer != nullptr)
    {
        if (!buffer)
        {
            m_data = new uint8_t[size];
        }
    }

    virtual ~cByteArray ()
    {
        if (!m_extData)
        {
            delete[] m_data;
            m_data = nullptr;
        }
    }

    const uint8_t* data () const
    {
        return m_data;
    }

    void append (const void* p, size_t len)
    {
        if (m_currPos + len > m_capacity)
            resize (m_capacity + len);
        std::memcpy (&m_data[m_currPos], p, len);
        m_currPos += len;
    }

    cByteArray &operator<<(uint8_t val)
    {
        append (&val, sizeof(val));
        return *this;
    }

    cByteArray &operator<<(uint16_t val)
    {
        append (&val, sizeof(val));
        return *this;
    }

    cByteArray &operator<<(uint32_t val)
    {
        append (&val, sizeof(val));
        return *this;
    }

    cByteArray &operator<<(uint64_t val)
    {
        append (&val, sizeof(val));
        return *this;
    }

    uint8_t& operator [](size_t idx)
    {
        BUG_ON (idx >= m_capacity);
        return m_data[idx];
    }

    uint8_t operator [](size_t idx) const
    {
        BUG_ON (idx >= m_capacity);
        return m_data[idx];
    }

    size_t capacity () const
    {
        return m_capacity;
    }

    size_t size () const
    {
        return m_currPos;
    }

protected:
    void resize (size_t newLength)
    {
        if (!m_dynSize || m_extData)
            throw std::out_of_range ("resizing of fixed size array is not possible");

        newLength = alignLength (newLength);

        // nothing to do
        if (newLength == m_capacity)
            return;

        if (newLength < m_capacity)
        {
            // on truncation we keep the current buffer and reduce only the length
            m_capacity = newLength;
            if (m_currPos >= m_capacity)
                m_currPos = m_capacity;
        }
        else
        {
            uint8_t *newData = new uint8_t[newLength];
            if (m_data)
                std::memcpy (newData, m_data, m_capacity);
            delete[] m_data;
            m_data = newData;
            m_capacity = newLength;
        }
    }
    size_t alignLength (size_t len) const
    {
        size_t chunks = (len + m_chunkSize - 1) / m_chunkSize;
        if (!len) chunks++;
        return chunks * m_chunkSize;
    }

private:
    const size_t m_chunkSize;
    uint8_t* m_data;
    size_t   m_capacity;
    size_t   m_currPos;  // m_currPos == m_capacity --> full
    bool     m_dynSize;  // false, if resizing is not allowd
    bool     m_extData;  // true, if m_data is not managed within this class

#ifdef WITH_UNITTESTS
public:
    static void unitTest ()
    {
        {
            cByteArray obj (1);
            BUG_ON (obj.capacity() != obj.m_chunkSize);
            BUG_ON (obj.size() != 0);
        }
        {
            cByteArray obj (cByteArray::DEFAULT_CHUNK);
            BUG_ON (obj.capacity() != obj.m_chunkSize);
            BUG_ON (obj.size() != 0);
        }
        {
            cByteArray obj (cByteArray::DEFAULT_CHUNK-1);
            BUG_ON (obj.capacity() != obj.m_chunkSize);
            BUG_ON (obj.size() != 0);
        }
        {
            cByteArray obj (cByteArray::DEFAULT_CHUNK+1);
            BUG_ON (obj.capacity() != (obj.m_chunkSize*2));
            BUG_ON (obj.size() != 0);
        }
        {
            cByteArray obj;
            BUG_ON (obj.capacity() != 0);
            BUG_ON (obj.size() != 0);
            obj << (uint8_t)1;
            BUG_ON (obj.capacity() != obj.m_chunkSize);
            BUG_ON (obj.size() != 1);
            BUG_ON (obj[0] != 1);
            obj << (uint16_t)0x0202;
            BUG_ON (obj.capacity() != obj.m_chunkSize);
            BUG_ON (obj.size() != 3);
            BUG_ON (obj[0] != 1);
            BUG_ON (obj[1] != 2);
            BUG_ON (obj[2] != 2);
            obj << (uint32_t)0x03030303;
            BUG_ON (obj.capacity() != obj.m_chunkSize);
            BUG_ON (obj.size() != 7);
            BUG_ON (obj[0] != 1);
            BUG_ON (obj[1] != 2);
            BUG_ON (obj[2] != 2);
            BUG_ON (obj[3] != 3);
            BUG_ON (obj[4] != 3);
            BUG_ON (obj[5] != 3);
            BUG_ON (obj[6] != 3);
            obj << (uint64_t)0x0404040404040404;
            BUG_ON (obj.capacity() != obj.m_chunkSize);
            BUG_ON (obj.size() != 15);
            BUG_ON (obj[0] != 1);
            BUG_ON (obj[1] != 2);
            BUG_ON (obj[2] != 2);
            BUG_ON (obj[3] != 3);
            BUG_ON (obj[4] != 3);
            BUG_ON (obj[5] != 3);
            BUG_ON (obj[6] != 3);
            BUG_ON (obj[7] != 4);
            BUG_ON (obj[8] != 4);
            BUG_ON (obj[9] != 4);
            BUG_ON (obj[10] != 4);
            BUG_ON (obj[11] != 4);
            BUG_ON (obj[12] != 4);
            BUG_ON (obj[13] != 4);
            BUG_ON (obj[14] != 4);
            for (size_t n = 15; n < obj.m_chunkSize; n++)
                obj << (uint8_t)9;
            BUG_ON (obj.capacity() != obj.m_chunkSize);
            BUG_ON (obj.size() != obj.m_chunkSize);
                obj << (uint8_t)10;
            BUG_ON (obj.capacity() != (obj.m_chunkSize*2));
            BUG_ON (obj.size() != (obj.m_chunkSize+1));
        }
        {
            cByteArray obj;
            obj << (uint8_t)1 << (uint16_t)0x0202 << (uint32_t)0x03030303 << (uint64_t)0x0404040404040404;
            BUG_ON (obj.capacity() != obj.m_chunkSize);
            BUG_ON (obj.size() != 15);
            BUG_ON (obj[0] != 1);
            BUG_ON (obj[1] != 2);
            BUG_ON (obj[2] != 2);
            BUG_ON (obj[3] != 3);
            BUG_ON (obj[4] != 3);
            BUG_ON (obj[5] != 3);
            BUG_ON (obj[6] != 3);
            BUG_ON (obj[7] != 4);
            BUG_ON (obj[8] != 4);
            BUG_ON (obj[9] != 4);
            BUG_ON (obj[10] != 4);
            BUG_ON (obj[11] != 4);
            BUG_ON (obj[12] != 4);
            BUG_ON (obj[13] != 4);
            BUG_ON (obj[14] != 4);
        }
        {
            cByteArray obj (nullptr, 1);
            BUG_ON (obj.capacity() != 1);
            BUG_ON (obj.size() != 0);
        }
        {
            bool catched = false;
            cByteArray obj (nullptr, 3);
            BUG_ON (obj.capacity() != 3);
            BUG_ON (obj.size() != 0);
            obj << (uint8_t)1;
            obj << (uint16_t)23;
            try
            {
                obj << (uint8_t)4;
            }
            catch (const std::out_of_range&)
            {
                catched = true;
            }
            BUG_ON (!catched);
            BUG_ON (obj.capacity() != 3);
            BUG_ON (obj.size() != 3);
        }
        {
            bool catched = false;
            uint8_t buffer[3];
            cByteArray obj (buffer, sizeof (buffer));
            BUG_ON (obj.capacity() != sizeof (buffer));
            BUG_ON (obj.size() != 0);
            obj << (uint8_t)1;
            obj << (uint16_t)23;
            try
            {
                obj << (uint8_t)4;
            }
            catch (const std::out_of_range&)
            {
                catched = true;
            }
            BUG_ON (!catched);
            BUG_ON (obj.capacity() != 3);
            BUG_ON (obj.size() != 3);
            BUG_ON (buffer[0] != 1);
#if HAVE_BIG_ENDIAN
            BUG_ON (buffer[1] != 0);
            BUG_ON (buffer[2] != 23);
#else
            BUG_ON (buffer[2] != 0);
            BUG_ON (buffer[1] != 23);
#endif
        }
    }
#endif

};

class cFixedByteArray : public cByteArray
{
public:
    cFixedByteArray (size_t length) : cByteArray (nullptr, length)
    {
    }
};

#endif
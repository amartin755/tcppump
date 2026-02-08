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


 // TEST-CODE!!!

#include <bitset>
#include <cstdlib>
#include <limits>

#include "parser.hpp"
#include "formatexception.hpp"
/*
class Protocol
{
    struct ProtocolSyntax *m_syntax;
    std::vector<ProtocolParameter> m_parameters;
};
*/

ProtocolParameter::ProtocolParameter (const char* name, size_t nameLen, const char* value, size_t valueLen,
    const std::vector<const ParameterSyntax*> mandatory, const std::vector<const ParameterSyntax*> optional)
    : m_name (name), m_nameLen (nameLen), m_strValue (value), m_strValueLen (valueLen), m_syntax (nullptr), m_isRandom (false), m_type (Type::Invalid)
{
    // we rely on the lexer to not call us with empty parameter name or value
    BUG_ON (!nameLen || !valueLen || !name || !value);

    for (const ParameterSyntax* s : mandatory)
    {
        if (std::strlen (s->syntax) == nameLen && std::strncmp (name, s->syntax, nameLen) == 0)
        {
            m_syntax = s;
            break;
        }
    }
    if (!m_syntax)
    {
        for (const ParameterSyntax* s : optional)
        {
            if (std::strlen (s->syntax) == nameLen && std::strncmp (name, s->syntax, nameLen) == 0)
            {
                m_syntax = s;
                break;
            }
        }
    }

    if (!m_syntax)
        throw FormatException (exParUnused, name, nameLen);

    // determine number of possible types for the current value
    size_t typeCnt = std::bitset<sizeof(m_syntax->type)> (m_syntax->type).count();

    if (m_syntax->type & Type::Integer)
    {
        // a generic integer must have an explicit range
        BUG_ON (!m_syntax->min || !m_syntax->max);

        // get range from syntax and convert to integer
        uint64_t min = std::strtoull (m_syntax->min, nullptr, 0);
        uint64_t max = std::strtoull (m_syntax->max, nullptr, 0);

        try
        {
            // read user provided value and convert to integer and check syntax and range
            m_value = getAndCheckIntegerValue (min, max);
            m_type = Type::Integer;
        }
        catch (...)
        {
            // only throw if there is no other types to try
            if (typeCnt-- == 1) throw;
        }
    }
    else if (m_syntax->type & Type::Bit)
    {
        try
        {
            m_value = getAndCheckIntegerValue (0, 1);
            m_type = Type::Bit;
        }
        catch (...)
        {
            // only throw if there is no other types to try
            if (typeCnt-- == 1) throw;
        }
    }
    else if (m_syntax->type & Type::Int8)
    {
        try
        {
            m_value = getAndCheckIntegerValue (0, std::numeric_limits<uint8_t>::max());
            m_type = Type::Int8;
        }
        catch (...)
        {
            // only throw if there is no other types to try
            if (typeCnt-- == 1) throw;
        }
    }
    else if (m_syntax->type & Type::Int16)
    {
        try
        {
            m_value = getAndCheckIntegerValue (0, std::numeric_limits<uint16_t>::max());
            m_type = Type::Int16;
        }
        catch (...)
        {
            // only throw if there is no other types to try
            if (typeCnt-- == 1) throw;
        }
    }
    else if (m_syntax->type & Type::Int32)
    {
        try
        {
            m_value = getAndCheckIntegerValue (0, std::numeric_limits<uint32_t>::max());
            m_type = Type::Int32;
        }
        catch (...)
        {
            // only throw if there is no other types to try
            if (typeCnt-- == 1) throw;
        }
    }
    else if (m_syntax->type & Type::Int64)
    {
        try
        {
            m_value = getAndCheckIntegerValue (0, std::numeric_limits<uint64_t>::max());
            m_type = Type::Int64;
        }
        catch (...)
        {
            // only throw if there is no other types to try
            if (typeCnt-- == 1) throw;
        }
    }
    else if (m_syntax->type & Type::Float)
    {
        double min, max;
        // a generic float must have an explicit range
        BUG_ON (!m_syntax->min || !m_syntax->max);

        // get range from syntax and convert to double
        min = std::strtod (m_syntax->min, nullptr);
        max = std::strtod (m_syntax->max, nullptr);

        // read user provided value and convert to double and check syntax and range
        char* end;
        errno = 0;
        double v = std::strtod (m_strValue, &end);
        if (end != (m_strValue + m_strValueLen))
        {
            throw FormatException (exParFormat, m_strValue, (int)m_strValueLen);
        }
        else if (errno == ERANGE || v < min || v > max)
        {
            throw FormatException (exParRange, m_strValue, (int)m_strValueLen);
        }
        m_value = v;
        m_type = Type::Float;
    }
    else if (m_syntax->type & Type::Mac)
    {
        try
        {
            m_value.emplace<cMacAddress> (m_strValue, m_strValueLen);
            m_type = Type::Mac;
        }
        catch(const std::exception&)
        {
            throw FormatException (exParFormat, m_strValue, (int)m_strValueLen);
        }
    }
    else if (m_syntax->type & Type::IP4)
    {
        try
        {
            m_value.emplace<cIPv4> (m_strValue, m_strValueLen);
            m_type = Type::IP4;
        }
        catch(const std::exception&)
        {
            throw FormatException (exParFormat, m_strValue, (int)m_strValueLen);
        }
    }
    else if (m_syntax->type & Type::IP6)
    {
        try
        {
            m_value.emplace<cIPv6> (m_strValue, m_strValueLen);
            m_type = Type::IP6;
        }
        catch(const std::exception&)
        {
            throw FormatException (exParFormat, m_strValue, (int)m_strValueLen);
        }
    }
    else if (m_syntax->type & Type::UUID)
    {
        if (isQuotedString ())
        {
            std::string uuidAsString (m_strValue + 1, m_strValueLen - 2);
            m_value = cUUID::fromString (uuidAsString);
            m_type = Type::UUID;
        }
        else
        {
            throw FormatException (exParFormat, m_strValue, (int)m_strValueLen);
        }
    }
    else if (m_syntax->type & Type::Nested)
    {
        if (*m_strValue != '<')
            throw FormatException (exParFormat, m_strValue, (int)m_strValueLen);

        m_value = std::make_unique<const Protocol> ();         //TODO 
        m_type = Type::Nested;
    }
    else if (m_syntax->type & Type::Bytestream)
    {
        size_t len = 0;
        size_t min = static_cast<size_t>(m_syntax->min ? std::strtoull (m_syntax->min, nullptr, 0) : 0);
        size_t max = static_cast<size_t>(m_syntax->max ? std::strtoull (m_syntax->max, nullptr, 0) : 1024*1024); // 1MiB should be enough

        if (isQuotedString ())
        {
            // remove quotes
            m_strValueLen -= 2;
            m_strValue++;

            len = m_strValueLen;
            m_value = m_strValue;
        }
        else
        {
            // convert hex string to binary data
            auto ptr = std::unique_ptr<std::vector<uint8_t>>(cParseHelper::hexStringToBin (m_strValue, m_strValueLen));
            if (!ptr)
                throw FormatException (exParFormat, m_strValue, (int)m_strValueLen);
            m_value = std::move(ptr);
            len = ptr->size();
        }

        if (len < min || len > max)
        {
            m_type = Type::Invalid;
            throw FormatException (exParRange, m_strValue, (int)m_strValueLen);
        }
    }

    // if m_type is not zero, we either forgot to implement a handler for a particular type
    // or a invalid type was used in the syntax definition
    BUG_ON (m_type == Type::Invalid);
    BUG_ON (m_value.index() == std::variant_npos);
}

int ProtocolParameter::isRandomInteger (uint64_t& min, uint64_t& max) const
    {
        if (!m_strValueLen || *m_strValue != '*')
            return -1;

        if (m_strValueLen == 1)
            return 0;

        if (!cParseHelper::range (m_strValue + 1, m_strValueLen - 1, 0, min, max))
            throw FormatException (exParFormat, m_strValue+1, (int)m_strValueLen-1);

        return 1;
    }

uint64_t ProtocolParameter::getAndCheckIntegerValue (uint64_t min, uint64_t max) const
{
    char* end;
    errno = 0;
    unsigned long long v = std::strtoull (m_strValue, &end, 0);
    if (end != (m_strValue + m_strValueLen))
    {
        throw FormatException (exParFormat, m_strValue, (int)m_strValueLen);
    }
    else if (errno == ERANGE || v < min || v > max)
    {
        throw FormatException (exParRange, m_strValue, (int)m_strValueLen);
    }
    return (uint64_t)v;
}


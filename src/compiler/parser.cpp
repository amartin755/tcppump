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

#include "bug.hpp"
#include "parser.hpp"
#include "formatexception.hpp"
#include "parsehelper.hpp"

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

    // unknown parameter
    if (!m_syntax)
        throw FormatException (exParUnused, name, nameLen);

    // determine number of possible types for the current value
    size_t typeCnt = std::bitset<sizeof(m_syntax->type)*8> (m_syntax->type).count();

    if (m_syntax->type & Type::Integer)
    {
        typeCnt--;

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
            // only throw if there are no other types to try
            if (typeCnt == 0) throw;
        }
    }
    else if (m_syntax->type & Type::Bit)
    {
        typeCnt--;
        try
        {
            m_value = getAndCheckIntegerValue (0, 1);
            m_type = Type::Bit;
        }
        catch (...)
        {
            // only throw if there are no other types to try
            if (typeCnt == 0) throw;
        }
    }
    else if (m_syntax->type & Type::Int8)
    {
        typeCnt--;
        try
        {
            m_value = getAndCheckIntegerValue (0, std::numeric_limits<uint8_t>::max());
            m_type = Type::Int8;
        }
        catch (...)
        {
            // only throw if there are no other types to try
            if (typeCnt == 0) throw;
        }
    }
    else if (m_syntax->type & Type::Int16)
    {
        typeCnt--;
        try
        {
            m_value = getAndCheckIntegerValue (0, std::numeric_limits<uint16_t>::max());
            m_type = Type::Int16;
        }
        catch (...)
        {
            // only throw if there are no other types to try
            if (typeCnt == 0) throw;
        }
    }
    else if (m_syntax->type & Type::Int32)
    {
        typeCnt--;
        try
        {
            m_value = getAndCheckIntegerValue (0, std::numeric_limits<uint32_t>::max());
            m_type = Type::Int32;
        }
        catch (...)
        {
            // only throw if there are no other types to try
            if (typeCnt == 0) throw;
        }
    }
    else if (m_syntax->type & Type::Int64)
    {
        typeCnt--;
        try
        {
            m_value = getAndCheckIntegerValue (0, std::numeric_limits<uint64_t>::max());
            m_type = Type::Int64;
        }
        catch (...)
        {
            // only throw if there are no other types to try
            if (typeCnt == 0) throw;
        }
    }
    if (m_syntax->type & Type::Float)
    {
        typeCnt--;
        double min, max;
        // a generic float must have an explicit range
        BUG_ON (!m_syntax->min || !m_syntax->max);

        // get range from syntax and convert to double
        min = std::strtod (m_syntax->min, nullptr);
        max = std::strtod (m_syntax->max, nullptr);

        // read user provided value and convert to double and check syntax and range
        char* end;
        double v = std::strtod (m_strValue, &end);
        // only throw if there are no other types to try
        if (typeCnt == 0)
        {
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
    }
    if (m_syntax->type & Type::Mac)
    {
        typeCnt--;
        try
        {
            std::string s = checkForRandom<cMacAddress> ();
            m_value.emplace<cMacAddress> (s);
            m_type = Type::Mac;
        }
        catch(const std::exception&)
        {
            // only throw if there are no other types to try
            if (typeCnt == 0)
                throw FormatException (exParFormat, m_strValue, (int)m_strValueLen);
        }
    }
    if (m_syntax->type & Type::IP4)
    {
        typeCnt--;
        try
        {
            std::string s = checkForRandom<cIPv4> ();
            m_value.emplace<cIPv4> (s);
            m_type = Type::IP4;
        }
        catch(const std::exception&)
        {
            // only throw if there are no other types to try
            if (typeCnt == 0)
                throw FormatException (exParFormat, m_strValue, (int)m_strValueLen);
        }
    }
    if (m_syntax->type & Type::IP6)
    {
        typeCnt--;
        try
        {
            std::string s = checkForRandom<cIPv6> ();
            m_value.emplace<cIPv6> (s);
            m_type = Type::IP6;
        }
        catch(const std::exception&)
        {
            // only throw if there are no other types to try
            if (typeCnt == 0)
                throw FormatException (exParFormat, m_strValue, (int)m_strValueLen);
        }
    }
    if (m_syntax->type & Type::UUID)
    {
        typeCnt--;
        if (isQuotedString ())
        {
            std::string uuidAsString (m_strValue + 1, m_strValueLen - 2);
            m_value = cUUID::fromString (uuidAsString);
            m_type = Type::UUID;
        }
        else
        {
            // only throw if there are no other types to try
            if (typeCnt == 0)
                throw FormatException (exParFormat, m_strValue, (int)m_strValueLen);
        }
    }
    if (m_syntax->type & Type::Nested)
    {
        typeCnt--;
        if (*m_strValue != '<')
            throw FormatException (exParFormat, m_strValue, (int)m_strValueLen);

        m_value = std::make_unique<const Protocol> ();         //TODO
        m_type = Type::Nested;
    }
    if (m_syntax->type & Type::Bytestream)
    {
        typeCnt--;
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

            // TODO in case of random we must create an empty vector
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

#ifdef WITH_UNITTESTS

void ProtocolParameter::unitTest ()
{
    static const std::vector<testcase_t<cMacAddress>> tests = 
    {
        {
            "mac",
            "12:34:56:78:9a:bc",
            false,
            false,
            "12:34:56:78:9a:bc",
            {
                "12:34:56:78:9a:bc",
                "12:34:56:78:9a:bc"
            }
        },
        {
            "mac",
            "*",
            false,
            true,
            "0:0:0:0:0:0",
            {
                "00:01:02:03:04:05",
                "06:07:08:09:0a:0b"
            }
        },
        {
            "mac",
            "11:*:33:44:*[10-12]:66",
            false,
            true,
            "11:0:33:44:0:66",
            {
                "11:0:33:44:11:66",
                "11:2:33:44:10:66",
                "11:4:33:44:12:66",
                "11:6:33:44:11:66"
            }
        },
        {
            "mac",
            "11:22:33:44:*[13-12]:66",
            true,
            false,
            cMacAddress(),
            {}
        },
        {
            "mac",
            "11:22:33:44:*[13-100]:66",
            true,
            false,
            cMacAddress(),
            {}
        },
        {
            "mac",
            "11:22:33:44:*[0-ff]:66",
            false,
            true,
            "11:22:33:44:00:66",
            {
                "11:22:33:44:00:66"
            }
        },
        {
            "mac",
            "*[0-f]:*[10-1f]:*[20-2f]:*[30-3f]:*[40-4f]:*[50-5f]",
            false,
            true,
            "0:0:0:0:0:0",
            {
                "00:11:22:33:44:55"
            }
        }
    };
    runTestCase<cMacAddress> (tests);

    static const std::vector<testcase_t<cIPv4>> ipv4tests = 
    {
        {"ip4", ".", true, false, cIPv4(), {}},
        {"ip4", "..", true, false, cIPv4(), {}},
        {"ip4", "1..3.4", true, false, cIPv4(), {}},
        {"ip4", "*[2-4].*.3.", true, false, cIPv4(), {}},
        {"ip4", "*[300-400].*.3.4", true, false, cIPv4(), {}},
        {"ip4", "*.*.*.*.*", true, false, cIPv4(), {}},
        {"ip4", "*[0x2-0x4].2.3.4", true, false, cIPv4(), {}},
        {"ip4", "1.*[0x2-0x4].3.4", true, false, cIPv4(), {}},
        {"ip4", "1.2.*[0x2-0x4].4", true, false, cIPv4(), {}},
        {"ip4", "1.2.3.*[0x2-0x4]", true, false, cIPv4(), {}},
        {"ip4", "[2-4].2.3.4", true, false, cIPv4(), {}},
        {"ip4", "1.[2-4].3.4", true, false, cIPv4(), {}},
        {"ip4", "1.2.[2-4].4", true, false, cIPv4(), {}},
        {"ip4", "1.2.3.[2-4]", true, false, cIPv4(), {}},
        {"ip4", "1.2.3.*", false, true, "1.2.3.0", {"1.2.3.0", "1.2.3.1"}},
        {"ip4", "1.2.*.4", false, true, "1.2.0.4", {"1.2.0.4", "1.2.1.4"}},
        {"ip4", "1.2.*.*", false, true, "1.2.0.0", {"1.2.0.1", "1.2.2.3"}},
        {"ip4", "1.*.3.4", false, true, "1.0.3.4", {"1.0.3.4", "1.1.3.4"}},
        {"ip4", "1.*.3.*", false, true, "1.0.3.0", {"1.0.3.1", "1.2.3.3"}},
        {"ip4", "1.*.*.4", false, true, "1.0.0.4", {"1.0.1.4", "1.2.3.4"}},
        {"ip4", "1.*.*.*", false, true, "1.0.0.0", {"1.0.1.2", "1.3.4.5"}},
        {"ip4", "*.2.3.4", false, true, "0.2.3.4", {"0.2.3.4", "1.2.3.4"}},
        {"ip4", "*.2.3.*", false, true, "0.2.3.0", {"0.2.3.1", "2.2.3.3"}},
        {"ip4", "*.2.*.4", false, true, "0.2.0.4", {"0.2.1.4", "2.2.3.4"}},
        {"ip4", "*.2.*.*", false, true, "0.2.0.0", {"0.2.1.2", "3.2.4.5"}},
        {"ip4", "*.*.3.4", false, true, "0.0.3.4", {"0.1.3.4", "2.3.3.4"}},
        {"ip4", "*.*.3.*", false, true, "0.0.3.0", {"0.1.3.2", "3.4.3.5"}},
        {"ip4", "*.*.*.4", false, true, "0.0.0.4", {"0.1.2.4", "3.4.5.4"}},
        {"ip4", "1.2.3.*[10-11]", false, true, "1.2.3.0", {"1.2.3.10", "1.2.3.11"}},
        {"ip4", "1.2.*[10-11].4", false, true, "1.2.0.4", {"1.2.10.4", "1.2.11.4"}},
        {"ip4", "1.2.*[10-12].*[10-12]", false, true, "1.2.0.0", {"1.2.10.11", "1.2.12.10"}},
        {"ip4", "1.*[10-11].3.4", false, true, "1.0.3.4", {"1.10.3.4", "1.11.3.4"}},
        {"ip4", "1.*[10-11].3.*[10-11]", false, true, "1.0.3.0", {"1.10.3.11", "1.10.3.11"}},
        {"ip4", "1.*[10-12].*[10-12].4", false, true, "1.0.0.4", {"1.10.11.4", "1.12.10.4"}},
        {"ip4", "1.*[10-11].*[10-11].*[10-11]", false, true, "1.0.0.0", {"1.10.11.10", "1.11.10.11"}},
        {"ip4", "*[10-11].2.3.4", false, true, "0.2.3.4", {"10.2.3.4", "11.2.3.4"}},
        {"ip4", "*[10-11].2.3.*[10-13]", false, true, "0.2.3.0", {"10.2.3.11", "10.2.3.13"}},
        {"ip4", "*[10-11].2.*[10-13].4", false, true, "0.2.0.4", {"10.2.11.4", "10.2.13.4"}},
        {"ip4", "*[10-11].2.*[10-11].*[10-11]", false, true, "0.2.0.0", {"10.2.11.10", "11.2.10.11"}},
        {"ip4", "*[10-11].*[10-11].3.4", false, true, "0.0.3.4", {"10.11.3.4", "10.11.3.4"}},
        {"ip4", "*[10-11].*[10-11].3.*[10-11]", false, true, "0.0.3.0", {"10.11.3.10", "11.10.3.11"}},
        {"ip4", "*[10-11].*[10-11].*[10-11].4", false, true, "0.0.0.4", {"10.11.10.4", "11.10.11.4"}},
        {"ip4", "1.2.3.4", false, false, "1.2.3.4", {"1.2.3.4", "1.2.3.4"}},
        {"ip4", "*.*.3.*[50-60]", false, true, "0.0.3.0", {"0.1.3.52", "3.4.3.55"}},
        {"ip4", "*.*.*.*", false, true, "0.0.0.0", {"0.1.2.3", "4.5.6.7"}},
        {"ip4", "*", false, true, "0.0.0.0", {"0.0.0.0", "0.0.0.1"}}
    };
    runTestCase<cIPv4> (ipv4tests);

    static const std::vector<testcase_t<cIPv6>> ipv6tests = 
    {
        {
            "ip6",
            "fe80::1ff:fe23:4577:890a",
            false,
            false,
            "fe80::1ff:fe23:4577:890a",
            {
                "fe80::1ff:fe23:4577:890a",
                "fe80::1ff:fe23:4577:890a"
            }
        },
        {
            "ip6",
            "fe80::1ff:*:4577:890a",
            false,
            true,
            "fe80::1ff:0:4577:890a",
            {
                "fe80::1ff:0000:4577:890a",
                "fe80::1ff:0001:4577:890a"
            }
        },
        {
            "ip6",
            "*::1ff:fe23:4577:*[1000-2000]",
            false,
            true,
            "0::1ff:fe23:4577:0",
            {
                "0000::1ff:fe23:4577:1001",
                "0002::1ff:fe23:4577:1003"
            }
        },
        {
            "ip6",
            "*",
            false,
            true,
            "0:0:0:0:0:0:0:0",
            {
                "0000:0001:0002:0003:0004:0005:0006:0007"
            }
        }
    };
    runTestCase<cIPv6> (ipv6tests);
}

template<typename T>
void ProtocolParameter::runTestCase(const std::vector<testcase_t<T>>& tests)
{
    bool catched;
    size_t n = 0;
    for (const auto& t : tests)
    {
        try
        {
            cRandom::setCounterMode (0);
            catched = false;
            ProtocolParameter obj (t.name.c_str(), t.name.size(), t.value.c_str(), t.value.size(), PR_RAW.mandatory, PR_RAW.optional);
            if (t.willThrow)
                BUG ("expected to throw");
            BUG_ON (t.isRandom != obj.m_isRandom);
            BUG_ON (std::memcmp (t.expInternalValue.getAsArray(), std::get<T> (obj.m_value).getAsArray(), t.expInternalValue.size()));

            for (const auto& expValue : t.expExternalValues)
            {
                const uint8_t *value;
                if constexpr (std::is_same_v<T, cMacAddress>)
                    value = obj.asMac().getAsArray();
                else if constexpr (std::is_same_v<T, cIPv4>)
                    value = obj.asIPv4().getAsArray();
                else if constexpr (std::is_same_v<T, cIPv6>)
                    value = obj.asIPv6().getAsArray();
                else
                    static_assert(sizeof(T) == 0, "Unsupported type");

                BUG_ON (std::memcmp(expValue.getAsArray(), value, t.expInternalValue.size()));
            }
        }
        catch(...)
        {
            catched = true;
        }
        BUG_ON (t.willThrow != catched);
        
        n++;
    }
}

#endif

/*

Random values:
Integer:
"*"
"*[min-max]"
min, max: range of the integer type
--> 2-16 bytes

MAC:
"*"
"*:*:*:*:*:*"
"*[min-max]:*[min-max]:*[min-max]:*[min-max]:*[min-max]:*[min-max]"
min, max: 0-ff
--> 12 bytes

IPv4
"*"
"*.*.*.*"
"*[min-max].*[min-max].*[min-max].*[min-max]"
min, max: 0-255
--> 8 bytes

IPv6
"*"
"*:*:*:*:*:*:*:*"
"*[min-max]:*[min-max]:*[min-max]:*[min-max]:*[min-max]:*[min-max]:*[min-max]:*[min-max]"
min, max: 0-ffff
--> 32 bytes

Stream:
"*"
"*len" == shortcut for "*[minlen-maxlen]"
min, max: 0-1MiB
--> 8 bytes

*/
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
            if (!checkForRandom<uint64_t>(min, max))
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
            if (!checkForRandom<uint8_t>(0, 1))
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
            constexpr uint8_t min = std::numeric_limits<uint8_t>::min();
            constexpr uint8_t max = std::numeric_limits<uint8_t>::max();
            if (!checkForRandom<uint8_t>(min, max))
                m_value = getAndCheckIntegerValue (min, max);
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
            constexpr uint16_t min = std::numeric_limits<uint16_t>::min();
            constexpr uint16_t max = std::numeric_limits<uint16_t>::max();
            if (!checkForRandom<uint16_t>(min, max))
                m_value = getAndCheckIntegerValue (min, max);
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
            constexpr uint32_t min = std::numeric_limits<uint32_t>::min();
            constexpr uint32_t max = std::numeric_limits<uint32_t>::max();
            if (!checkForRandom<uint32_t>(min, max))
                m_value = getAndCheckIntegerValue (min, max);
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
            constexpr uint64_t min = std::numeric_limits<uint64_t>::min();
            constexpr uint64_t max = std::numeric_limits<uint64_t>::max();
            if (!checkForRandom<uint64_t>(min, max))
                m_value = getAndCheckIntegerValue (min, max);
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

        // a generic float must have an explicit range
        BUG_ON (!m_syntax->min || !m_syntax->max);

        // get range from syntax and convert to double
        const double min = std::strtod (m_syntax->min, nullptr);
        const double max = std::strtod (m_syntax->max, nullptr);

        try
        {
            // read user provided value and convert to integer and check syntax and range
            if (!checkForRandom<double>(min, max))
                m_value = getAndCheckDoubleValue (min, max);
            m_type = Type::Float;
        }
        catch (...)
        {
            // only throw if there are no other types to try
            if (typeCnt == 0) throw;
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
        const size_t min = static_cast<size_t>(m_syntax->min ? std::strtoull (m_syntax->min, nullptr, 0) : 0);
        const size_t max = static_cast<size_t>(m_syntax->max ? std::strtoull (m_syntax->max, nullptr, 0) : 1024*1024); // 1MiB should be enough

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

double ProtocolParameter::getAndCheckDoubleValue (double min, double max) const
{
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
    return v;
}

#ifdef WITH_UNITTESTS


static constexpr ParameterSyntax PAR_UNIT_I8   = {"i8",   "", Int8};
static constexpr ParameterSyntax PAR_UNIT_I16  = {"i16",  "", Int16};
static constexpr ParameterSyntax PAR_UNIT_I32  = {"i32",  "", Int32};
static constexpr ParameterSyntax PAR_UNIT_I64  = {"i64",  "", Int64};
static constexpr ParameterSyntax PAR_UNIT_STR  = {"str",  "", Bytestream};
static constexpr ParameterSyntax PAR_UNIT_IP4  = {"ip4",  "", IP4};
static constexpr ParameterSyntax PAR_UNIT_IP6  = {"ip6",  "", IP6};
static constexpr ParameterSyntax PAR_UNIT_MAC  = {"mac",  "", Mac};
static constexpr ParameterSyntax PAR_UNIT_FLT  = {"float", "", Float, "1.0", "3.14"};
static constexpr ParameterSyntax PAR_UNIT_INT  = {"int",   "", Integer, "100", "200000"};
static constexpr ParameterSyntax PAR_UNIT_STRR = {"str_range", "", Bytestream, "32", "100"};
static ProtocolSyntax PR_UNIT = {
    "unit",
    "",
    {},
    {
        &PAR_UNIT_I8,
        &PAR_UNIT_I16,
        &PAR_UNIT_I32,
        &PAR_UNIT_I64,
        &PAR_UNIT_STR,
        &PAR_UNIT_IP4,
        &PAR_UNIT_IP6,
        &PAR_UNIT_MAC,
        &PAR_UNIT_FLT,
        &PAR_UNIT_INT,
        &PAR_UNIT_STRR
    }
};

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

    static const std::vector<testcase_t<uint8_t>> i8tests = 
    {
        {"i8", "0", false, false, 0, {0}},
        {"i8", "255", false, false, 255, {255, 255}},
        {"i8", "42", false, false, 42, {42}},
        {"i8", "256", true, false, 0, {}},
        {"i8", "0x0", false, false, 0, {0}},
        {"i8", "0xff", false, false, 255, {255}},
        {"i8", "0x42", false, false, 0x42, {0x42}},
        {"i8", "0x100", true, false, 0, {}},

        {"i8", "*", false, true, 0, {0, 1}},
        {"i8", "*[100-101]", false, true, 0, {100, 101, 100}},
        {"i8", "*[0x64-101]", false, true, 0, {100, 101, 100}},
        {"i8", "*[0x64-0x65]", false, true, 0, {100, 101, 100}},
        {"i8", "*[101-100]", true, true, 0, {}},
        {"i8", "*[255-256]", true, true, 0, {}},
        {"i8", "*[256-257]", true, true, 0, {}}

        // TODO syntax errors
    };
    runTestCase<uint8_t> (i8tests);

    static const std::vector<testcase_t<uint16_t>> i16tests = 
    {
        {"i16", "0", false, false, 0, {0}},
        {"i16", "65535", false, false, std::numeric_limits<uint16_t>::max(), {std::numeric_limits<uint16_t>::max(), std::numeric_limits<uint16_t>::max()}},
        {"i16", "4200", false, false, 4200, {4200}},
        {"i16", "65536", true, false, 0, {}},
        {"i16", "0x0", false, false, 0, {0}},
        {"i16", "0xffff", false, false, std::numeric_limits<uint16_t>::max(), {std::numeric_limits<uint16_t>::max()}},
        {"i16", "0x4200", false, false, 0x4200, {0x4200}},
        {"i16", "0x10000", true, false, 0, {}},

        {"i16", "*", false, true, 0, {0, 1}},
        {"i16", "*[1000-1001]", false, true, 0, {1000, 1001, 1000}},
        {"i16", "*[0x3e8-1001]", false, true, 0, {1000, 1001, 1000}},
        {"i16", "*[0x3e8-0x3e9]", false, true, 0, {1000, 1001, 1000}},
        {"i16", "*[1001-1000]", true, true, 0, {}},
        {"i16", "*[65535-65536]", true, true, 0, {}},
        {"i16", "*[65536-65537]", true, true, 0, {}}

        // TODO syntax errors
    };
    runTestCase<uint16_t> (i16tests);

    static const std::vector<testcase_t<uint32_t>> i32tests = 
    {
        {"i32", "0", false, false, 0, {0}},
        {"i32", "4294967295", false, false, std::numeric_limits<uint32_t>::max(), {std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max()}},
        {"i32", "42000000", false, false, 42000000, {42000000}},
        {"i32", "4294967296", true, false, 0, {}},
        {"i32", "0x0", false, false, 0, {0}},
        {"i32", "0xffffffff", false, false, std::numeric_limits<uint32_t>::max(), {std::numeric_limits<uint32_t>::max()}},
        {"i32", "0x42000000", false, false, 0x42000000, {0x42000000}},
        {"i32", "0x100000000", true, false, 0, {}},

        {"i32", "*", false, true, 0, {0, 1}},
        {"i32", "*[100000-100001]", false, true, 0, {100000, 100001, 100000}},
        {"i32", "*[0x186a0-100001]", false, true, 0, {100000, 100001, 100000}},
        {"i32", "*[0x186a0-0x186a1]", false, true, 0, {100000, 100001, 100000}},
        {"i32", "*[100001-100000]", true, true, 0, {}},
        {"i32", "*[4294967295-4294967296]", true, true, 0, {}},
        {"i32", "*[4294967296-4294967297]", true, true, 0, {}}

        // TODO syntax errors
    };
    runTestCase<uint32_t> (i32tests);

    static const std::vector<testcase_t<uint64_t>> i64tests = 
    {
        {"i64", "0", false, false, 0, {0}},
        {"i64", "18446744073709551615", false, false, std::numeric_limits<uint64_t>::max(), {std::numeric_limits<uint64_t>::max(), std::numeric_limits<uint64_t>::max()}},
        {"i64", "420000000000", false, false, 420000000000, {420000000000}},
        {"i64", "18446744073709551616", true, false, 0, {}},
        {"i64", "0x0", false, false, 0, {0}},
        {"i64", "0xffffffffffffffff", false, false, std::numeric_limits<uint64_t>::max(), {std::numeric_limits<uint64_t>::max()}},
        {"i64", "0x42000000", false, false, 0x42000000, {0x42000000}},
        {"i64", "0x10000000000000000", true, false, 0, {}},

        {"i64", "*", false, true, 0, {0, 1}},
        {"i64", "*[10000000000-10000000001]", false, true, 0, {10000000000, 10000000001, 10000000000}},
        {"i64", "*[0x2540BE400-10000000001]", false, true, 0, {10000000000, 10000000001, 10000000000}},
        {"i64", "*[0x2540BE400-0x2540BE401]", false, true, 0, {10000000000, 10000000001, 10000000000}},
        {"i64", "*[10000000001-10000000000]", true, true, 0, {}},
        {"i64", "*[18446744073709551615-18446744073709551616]", true, true, 0, {}},
        {"i64", "*[18446744073709551616-18446744073709551617]", true, true, 0, {}}

        // TODO syntax errors
    };
    runTestCase<uint64_t> (i64tests);

    static const std::vector<testcase_t<double>> dbltests = 
    {
        {"float", "0.0", true, false, 0, {0}},
        {"float", "1", false, false, 1.0, {1.0}},
        {"float", "1.0", false, false, 1.0, {1.0}},
        {"float", "3.14", false, false, 3.14, {3.14}},
        {"float", "3.15", true, false, 0, {}},
        {"float", "*", false, true, 1, {1}}
    };
    runTestCase<double> (dbltests);

    static const std::vector<testcase_t<uint32_t>> inttests = 
    {
        {"int", "0", true, false, 0, {}},
        {"int", "0x0", true, false, 0, {}},
        {"int", "99", true, false, 0, {}},
        {"int", "0x63", true, false, 0, {}},
        {"int", "200001", true, false, 0, {}},
        {"int", "0x30D41", true, false, 0, {}},
        {"int", "18446744073709551615", true, false, 0, {}},
        {"int", "0xffffffffffffffff", true, false, 0, {}},
        {"int", "18446744073709551616", true, false, 0, {}},
        {"int", "0x10000000000000000", true, false, 0, {}},

        {"int", "100", false, false, 100, {100}},
        {"int", "0x64", false, false, 100, {100}},
        {"int", "4200", false, false, 4200, {4200}},
        {"int", "0x1068", false, false, 4200, {4200}},
        {"int", "200000", false, false, 200000, {200000}},
        {"int", "0x30D40", false, false, 200000, {200000}},

        {"int", "*", false, true, 100, {100, 101}}/*,
        {"int", "*[1000-1001]", false, true, 0, {1000, 1001, 1000}},
        {"int", "*[0x3e8-1001]", false, true, 0, {1000, 1001, 1000}},
        {"int", "*[0x3e8-0x3e9]", false, true, 0, {1000, 1001, 1000}},
        {"int", "*[1001-1000]", true, true, 0, {}},
        {"int", "*[65535-65536]", true, true, 0, {}},
        {"int", "*[65536-65537]", true, true, 0, {}}*/

        // TODO syntax errors
    };
    runTestCase<uint32_t> (inttests);


    // TODO generic integer
    // TODO stream
    // TODO UUID
    // TODO nested
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
            ProtocolParameter obj (t.name.c_str(), t.name.size(), t.value.c_str(), t.value.size(), PR_UNIT.mandatory, PR_UNIT.optional);
            if (t.willThrow)
                BUG ("expected to throw");
            BUG_ON (t.isRandom != obj.m_isRandom);
            if constexpr (std::is_integral_v<T>)
                BUG_ON (t.expInternalValue != static_cast<T>(std::get<uint64_t> (obj.m_value)));
            else
                BUG_ON (t.expInternalValue != std::get<T> (obj.m_value));

            for (const auto& expValue : t.expExternalValues)
            {
                T value;
                if constexpr (std::is_same_v<T, cMacAddress>)
                    value.set(obj.asMac());
                else if constexpr (std::is_same_v<T, cIPv4>)
                    value.set(obj.asIPv4());
                else if constexpr (std::is_same_v<T, cIPv6>)
                    value.set(obj.asIPv6());
                else if constexpr (std::is_same_v<T, double>)
                    value = obj.get<T,T>();
                else
                    value = obj.get<T>();

                BUG_ON (value != expValue);
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
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

#include <variant>
#include <vector>
#include <cstdlib>
#include <memory>
#include <utility>
#include <tuple>

#include "syntax.hpp"
#include "ipaddress.hpp"
#include "macaddress.hpp"
#include "uuid.hpp"
#include "random.hpp"

class ProtocolParameter;

class Protocol
{
    struct ProtocolSyntax *m_syntax;
    std::vector<ProtocolParameter> m_parameters;
};

template<typename T>
struct checkForRandomTraits;

template<>
struct checkForRandomTraits<cIPv4> {
    using value_type = uint8_t;

    static constexpr char   delimiter   = '.';
    static constexpr int    base        = 10;
    static constexpr size_t maxTokenCnt = 4;
};

template<>
struct checkForRandomTraits<cIPv6> {
    using value_type = uint16_t;

    static constexpr char   delimiter   = ':';
    static constexpr int    base        = 16;
    static constexpr size_t maxTokenCnt = 8;
};

template<>
struct checkForRandomTraits<cMacAddress> {
    using value_type = uint8_t;

    static constexpr char   delimiter   = ':';
    static constexpr int    base        = 16;
    static constexpr size_t maxTokenCnt = 6;
};

class ProtocolParameter
{
public:
    ProtocolParameter (const char* name, size_t nameLen, const char* value, size_t valueLen,
        const std::vector<const ParameterSyntax*> mandatory, const std::vector<const ParameterSyntax*> optional);

    bool isNested () const
    {
        return m_type & Nested;
    }

    uint8_t asInt8 ()
    {
        if (m_isRandom)
            calcNextRandom<uint8_t> ();
        return static_cast<uint8_t> (std::get<uint64_t> (m_value));
    }
    uint16_t asInt16 ()
    {
        if (m_isRandom)
            calcNextRandom<uint16_t> ();
        return static_cast<uint16_t> (std::get<uint64_t> (m_value));
    }
    uint32_t asInt32 ()
    {
        if (m_isRandom)
            calcNextRandom<uint32_t> ();
        return static_cast<uint32_t> (std::get<uint64_t> (m_value));
    }
    uint64_t asInt64 ()
    {
        if (m_isRandom)
            calcNextRandom<uint64_t> ();
        return static_cast<uint64_t> (std::get<uint64_t> (m_value));
    }
    double asDouble ()
    {
        if (m_isRandom)
            calcNextRandom<double> ();
        return std::get<double> (m_value);
    }
    const cMacAddress& asMac ()
    {
        if (m_isRandom)
            calcNextRandom<cMacAddress, uint8_t> ();
        return std::get<cMacAddress> (m_value);
    }
    const cIPv4& asIPv4 ()
    {
        if (m_isRandom)
            calcNextRandom<cIPv4, uint8_t> ();
        return std::get<cIPv4> (m_value);
    }
    const cIPv6& asIPv6 ()
    {
        if (m_isRandom)
            calcNextRandom<cIPv6, uint16_t> ();
        return std::get<cIPv6> (m_value);
    }
    const cUUID& asUUID () const
    {
        return std::get<cUUID> (m_value);
    }
    const Protocol& asNested () const
    {
        return *std::get<std::unique_ptr<const Protocol>> (m_value);
    }
    const std::pair<const uint8_t*, size_t> asStream ()
    {
        if (m_isRandom)
            calcNextRandomStream ();

        // is our stream a string?
        if (std::holds_alternative <const char*>(m_value))
        {
            return std::pair <const uint8_t*, size_t>(
                reinterpret_cast<const uint8_t*>(m_strValue), m_strValueLen);
        }
        else
        {
            // no, then it must be a vector
            auto val = std::get <std::unique_ptr <std::vector<uint8_t>>>(m_value).get ();
            return std::pair <const uint8_t*, size_t> (val->data (), val->size ());
        }
    }

private:
    int isRandomInteger (uint64_t& min, uint64_t& max) const;
    uint64_t getAndCheckIntegerValue (uint64_t min, uint64_t max) const;
    bool isRandom (uint64_t min, uint64_t max);

    bool isQuotedString () const
    {
        return m_strValueLen >= 2 && *m_strValue == '"' && *(m_strValue + m_strValueLen - 1) == '"';
    }

    template<typename T>
    void calcNextRandom ()
    {
        const auto* p = std::get_if<std::pair<T, T>> (&m_randRanges);
        if (p)
        {
            const auto& [min, max] = *p;
            m_value = cRandom::rand<T> (min, max);
        }
        else
        {
            m_value = cRandom::rand<T> ();
        }
    }
    template<typename T, typename W>
    void calcNextRandom ()
    {
        auto& val = std::get<T> (m_value);
        const auto* randRanges = std::get_if<std::vector <std::tuple<int, W, W>>> (&m_randRanges);
        if (randRanges && randRanges->size())
        {
            for (const auto& [offset, min, max] : *randRanges)
            {
                val.setAt (offset, cRandom::rand<W> (min, max));
            }
        }
        else
        {
            val.setRandom();
        }
    }
    void calcNextRandomStream ()
    {
        const auto* range = std::get_if<std::pair<uint32_t, uint32_t>> (&m_randRanges);
        uint32_t minLen = 32;
        uint32_t maxLen = 32;
        if (range)
        {
            const auto& [min, max] = *range;
            minLen = min;
            maxLen = max;
        }
        // calc length of our random stream
        uint32_t nextLen = minLen == maxLen ? maxLen : cRandom::rand<uint32_t> (minLen, maxLen);

        // fill the array with random values
        // TODO this could be improved. 
        //      resize() unnecessarily copies and initializes the vector, but we will overwrite all values anyway.
        auto val = std::get <std::unique_ptr <std::vector<uint8_t>>>(m_value).get ();
        val->resize (nextLen);
        cRandom::rand (
            reinterpret_cast<void*>(val->data()), 
            static_cast<size_t> (nextLen));
    }
#if 0
    template<typename T>
    constexpr std::string_view emptyToken ()
    {
        if constexpr (std::is_same_v<T, uint16_t>)
            return "0000";
        else
            return "0";
    }
#endif
    // applies only for MAC and IP addresses
    template<typename T>
    std::string checkForRandom ()
    {
        using traits     = checkForRandomTraits<T>;
        using value_type = typename traits::value_type;
        constexpr char   delimiter   = traits::delimiter;
        constexpr int    base        = traits::base;
        constexpr size_t maxTokenCnt = traits::maxTokenCnt;

        std::string newValString; newValString.reserve (m_strValueLen); // the new string is never bigger then the original
        std::vector<std::string_view> tokens = cParseHelper::tokenize (m_strValue, m_strValueLen, delimiter);
        size_t index = 0, offset = 0;
        constexpr value_type max = std::numeric_limits<value_type>::max();
        constexpr std::string_view emptyToken = "0";

        // shortcut for "*"
        if (tokens.size() == 1 && tokens[0].size() == 1 && tokens[0][0] == '*')
        {
            m_isRandom = true;
            newValString = emptyToken;
            for (size_t n = 1; n < maxTokenCnt; n++)
            {
                newValString += delimiter;
                newValString += emptyToken;
            }
        }
        else
        {
            m_randRanges.emplace<std::vector <std::tuple<int, value_type, value_type>>>();
            for (const auto& token : tokens)
            {
                uint64_t randMin = 0, randMax = static_cast<uint64_t>(max);
                bool valid = false;
                if (token.size() && token[0] == '*')
                {
                    // range restricted random value
                    if (token.size() != 1)
                    {
                        uint64_t rangeMin, rangeMax;
                        if (cParseHelper::range (token.data() + 1, token.size() - 1, base, rangeMin, rangeMax) &&
                            (rangeMin <= max && rangeMax <= max) )
                        {
                            randMin = rangeMin;
                            randMax = rangeMax;
                            valid = true;
                        }
                    }
                    else
                    {
                        valid = true;
                    }

                }
                if (!token.size())
                    offset = maxTokenCnt - tokens.size();
                if (valid)
                {
                    newValString += emptyToken;
                    // create entry in m_randRanges
                    auto& val = std::get<std::vector <std::tuple<int, value_type, value_type>>> (m_randRanges);
                    val.emplace_back (index + offset, static_cast<value_type>(randMin), static_cast<value_type>(randMax));
                    m_isRandom = true;
                }
                else
                {
                    newValString += token;
                }
                // add delimiter if not last token
                if (++index < tokens.size())
                {
                    newValString += delimiter;
                }
            }
        }
        return newValString;
    }

    const char* m_name;
    size_t      m_nameLen;
    const char* m_strValue;
    size_t      m_strValueLen;

    const struct ParameterSyntax *m_syntax;
    bool m_isRandom;
    Type m_type;

    std::variant<
        uint64_t,
        double,
        cIPv4,
        cIPv6,
        cMacAddress,
        cUUID,
        std::unique_ptr <std::vector<uint8_t>>, // stream
        std::unique_ptr <const Protocol>,       // embedded
        const char* // string
    > m_value;

    std::variant<
        std::pair <uint8_t, uint8_t>,
        std::pair <uint16_t, uint16_t>,
        std::pair <uint32_t, uint32_t>,
        std::pair <uint64_t, uint64_t>,
        std::pair <double, double>,
        std::vector <std::tuple<int, uint8_t, uint8_t>>,    // ipv4, mac
        std::vector <std::tuple<int, uint16_t, uint16_t>>   // ipv6
    > m_randRanges;

    // TODO do we need unique id that it can be part of a map?


#ifdef WITH_UNITTESTS
public:
    static void unitTest ();
    template<typename T>
    struct testcase_t
    {
        const std::string name;
        const std::string value;
        bool willThrow;
        bool isRandom;
        T expInternalValue;
        std::vector<T> expExternalValues;
    };
private:
    template<typename T>
    static void runTestCase (const std::vector<testcase_t<T>>& testcases);
#endif

};


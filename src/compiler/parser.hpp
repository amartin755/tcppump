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


#ifndef PARSER_HPP_
#define PARSER_HPP_


#include <variant>
#include <vector>
#include <cstdlib>
#include <memory>
#include <utility>
#include <tuple>
#include <type_traits>

#include "syntax.hpp"
#include "ipaddress.hpp"
#include "macaddress.hpp"
#include "uuid.hpp"
#include "random.hpp"
#include "formatexception.hpp"


class Protocol;

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

template<typename>
inline constexpr bool dependent_false_v = false;

template<typename T>
inline constexpr bool is_value_type_v =
    std::is_integral_v<T>      || 
    std::is_same_v<T, double>;

class ProtocolParameter
{
    friend class Protocol;

public:
    ProtocolParameter (const char* name, size_t nameLen, const char* value, size_t valueLen,
        ParameterSyntaxArray mandatory, ParameterSyntaxArray optional, size_t position = -1);

    ~ProtocolParameter ();

    int key () const
    {
        return m_syntax->key;
    }

    size_t position () const
    {
        return m_position;
    }

    bool isNested () const
    {
        return m_type & Nested;
    }

    Type type () const
    {
        return m_type;
    }

    uint8_t asInt8 ()
    {
        return get<uint8_t>();
    }
    uint16_t asInt16 ()
    {
        return get<uint16_t>();
    }
    uint32_t asInt32 ()
    {
        return get<uint32_t>();
    }
    uint64_t asInt64 ()
    {
        return get<uint64_t>();
    }
    double asDouble ()
    {
        return get<double>();
    }
    const cMacAddress& asMac ()
    {
        return get<cMacAddress> ();
    }
    const cIPv4& asIPv4 ()
    {
        return get<cIPv4> ();
    }
    const cIPv6& asIPv6 ()
    {
        return get<cIPv6> ();
    }
    const cUUID& asUUID ()
    {
        return get<cUUID> ();
    }
    const Protocol& asNested () const
    {
        // TODO unify via get
        return *m_value.pNested;
    }
    const std::vector<uint8_t> asStream ()
    {
        return get<std::vector<uint8_t>> ();
    }

private:
    uint64_t getAndCheckIntegerValue (uint64_t min, uint64_t max) const;
    double getAndCheckDoubleValue (double min, double max) const;
    bool isRandom (uint64_t min, uint64_t max);
    bool checkForRandomStream (size_t rangeMin, size_t rangeMax);

    template<typename T>
    using get_set_t =
        std::conditional_t<is_value_type_v<T>, T, T&>;

    template<typename T>
    get_set_t<T> getRawValue()
    {
        if constexpr (std::is_integral_v<T>)
            return static_cast<T>(m_value.integer);
        else if constexpr (std::is_same_v<T, cMacAddress>)
            return *m_value.pMAC;
        else if constexpr (std::is_same_v<T, cIPv4>)
            return *m_value.pIPv4;
        else if constexpr (std::is_same_v<T, cIPv6>)
            return *m_value.pIPv6;
        else if constexpr (std::is_same_v<T, cUUID>)
            return *m_value.pUUID;
        else if constexpr (std::is_same_v<T, double>)
            return m_value.floatingPoint;
        else if constexpr (std::is_same_v<T, std::vector<uint8_t>>)
            return *m_value.pStream;
        else
            static_assert(dependent_false_v<T>, "T is unsupported");
    }

    template<typename T>
    get_set_t<T> get()
    {
        if (m_isRandom)
        {
            if constexpr (std::is_same_v<T, std::vector<uint8_t>>)
                calcNextRandomStream ();
            else
                calcNextRandom<T>();
        }

        return getRawValue <T> ();
    }    
    
    template<typename T>
    void set (get_set_t<T> value)
    {
        if constexpr (std::is_integral_v<T>)
            m_value.integer =  static_cast<uint64_t>(value);
        else if constexpr (std::is_same_v<T, cMacAddress>)
            *m_value.pMAC->set (value);
        else if constexpr (std::is_same_v<T, cIPv4>)
            *m_value.pIPv4->set (value);
        else if constexpr (std::is_same_v<T, cIPv6>)
            *m_value.pIPv6->set (value);
        else if constexpr (std::is_same_v<T, double>)
            m_value.floatingPoint = value;
        else
            static_assert(dependent_false_v<T>, "T is unsupported");
    }

    bool isQuotedString (const char* str, size_t len) const
    {
        return len >= 2 && *str == '"' && *(str + len - 1) == '"';
    }

    template<typename T>
    void calcNextRandom ()
    {
        if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>)
        {
            // internally we only have ranges for uint64_t (integers) and double (floating point numbers)
            using StorageType = std::conditional_t<std::is_integral_v<T>, std::uint64_t, double>;
            const auto* p = std::get_if<std::pair<StorageType, StorageType>> (&m_randRanges);
            if (p)
            {
                const auto& [min, max] = *p;
                this->set<StorageType> (static_cast<T>(cRandom::rand<StorageType> (min, max)));
            }
            else
            {
                this->set<StorageType> (static_cast<T>(cRandom::rand<StorageType> ()));
            }
        }
        else
        {
            // in case of IPv6 the elements are uint16_t, otherwise uint8_t
            using StorageType = std::conditional_t<std::is_same_v<T, cIPv6>, std::uint16_t, std::uint8_t>;
            auto& val = this->getRawValue<T> ();
            const auto* randRanges = std::get_if<std::vector <std::tuple<size_t, StorageType, StorageType>>> (&m_randRanges);
            if (randRanges && randRanges->size())
            {
                for (const auto& [offset, min, max] : *randRanges)
                {
                    val.setAt (offset, cRandom::rand<StorageType> (min, max));
                }
            }
            else
            {
                val.setRandom();
            }
        }
    }
    void calcNextRandomStream ()
    {
        const auto* range = std::get_if<std::pair<uint64_t, uint64_t>> (&m_randRanges);

        BUG_ON (!range);

        // calc length of our random stream
        const auto& [min, max] = *range;
        const uint32_t minLen = static_cast<uint32_t>(min);
        const uint32_t maxLen = static_cast<uint32_t>(max);
        uint32_t nextLen = minLen == maxLen ? maxLen : cRandom::rand<uint32_t> (minLen, maxLen);

        // fill the array with random values
        // TODO this could be improved. 
        //      resize() unnecessarily copies and initializes the vector, but we will overwrite all values anyway.
        auto& val = this->getRawValue <std::vector<uint8_t>> ();
        val.resize (nextLen);
        cRandom::rand (
            reinterpret_cast<void*>(val.data()), 
            static_cast<size_t> (nextLen));
    }

    template<typename T>
    bool checkForRandom (T rangeMin, T rangeMax)
    {
        using StorageType = std::conditional_t<std::is_integral_v<T>, std::uint64_t, T>;

        // no random value
        if (!m_strValueLen || *m_strValue != '*')
            return false;

        if (m_strValueLen == 1)
        {
            // random value without range restrictions
            m_randRanges.emplace<std::pair <StorageType, StorageType>> (rangeMin, rangeMax);
        }
        else
        {
            // random value with range e.g. *[12-42] (only supported for integers)
            if constexpr (std::is_integral_v<T>)
            {
                uint64_t min, max;
                if (!cParseHelper::range (m_strValue + 1, m_strValueLen - 1, 0, min, max))
                    throw FormatException (exParFormat, m_strValue+1, (int)m_strValueLen-1);

                // if there is a random range specified, it must not violate the values range
                if (static_cast<uint64_t>(min) < rangeMin || static_cast<uint64_t>(max) > rangeMax)
                    throw FormatException (exParRange, m_strValue, (int)m_strValueLen);
                m_randRanges.emplace<std::pair <StorageType, StorageType>> (static_cast<StorageType>(min), static_cast<StorageType>(max));
            }
            else
                throw FormatException (exParFormat, m_strValue, (int)m_strValueLen);
        }
        m_isRandom = true;
        return true;
    }

    // applies only for MAC and IP addresses
    template<typename T>
    std::string checkForRandom ()
    {
        using traits     = checkForRandomTraits<T>;
        using value_type = typename traits::value_type;
        constexpr char   delimiter   = traits::delimiter;
        constexpr int    base        = traits::base;
        constexpr size_t maxTokenCnt = traits::maxTokenCnt;

        std::string newValString; newValString.reserve (m_strValueLen); // the new string is never bigger than the original
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
            m_randRanges.emplace<std::vector <std::tuple<size_t, value_type, value_type>>>();
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
                    auto& val = std::get<std::vector <std::tuple<size_t, value_type, value_type>>> (m_randRanges);
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

    // FIXME we must get rid of this and work with a copy, because it could point to a no longer valid address
    //       after leaving the constructor. 
    const char* m_strValue;
    size_t      m_strValueLen;

    const struct ParameterSyntax *m_syntax;
    bool m_isRandom;
    Type m_type;
    size_t m_position;

    union TheValue
    {
        uint64_t               integer;
        double                 floatingPoint;
        cIPv4                 *pIPv4;
        cIPv6                 *pIPv6;
        cMacAddress           *pMAC;
        cUUID                 *pUUID;
        std::vector<uint8_t>  *pStream;
        const Protocol        *pNested;
    }m_value;

    std::variant<
        std::pair <uint64_t, uint64_t>,                        // integers
        std::pair <double, double>,
        std::vector <std::tuple<size_t, uint8_t, uint8_t>>,    // ipv4, mac
        std::vector <std::tuple<size_t, uint16_t, uint16_t>>   // ipv6
    > m_randRanges;



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
        Type expType;
        std::vector<T> expExternalValues;
    };
private:
    template<typename T>
    static void runTestCase (const std::vector<testcase_t<T>>& testcases);
#endif

};

class Protocol
{
public:
    Protocol (const char* instruction, bool acceptTrailingGarbage = false);

    ProtocolParameter* find (const ParameterSyntax* parameter, bool dontThrow = false)
    {
        return findParameter (parameter, nullptr, nullptr, dontThrow);
    }
    ProtocolParameter* findInRange (const ParameterSyntax* parameter,
        const ProtocolParameter* start, const ParameterSyntax* stop = nullptr, bool dontThrow = false)
    {
        return findParameter (parameter, start, stop, dontThrow);
    }

    template<typename T>
    using get_set_t =
        std::conditional_t<is_value_type_v<T>, T, const T&>;

    template<typename T>
    get_set_t<T> getValueOrDefault (const ParameterSyntax* parameter, const T& defaultValue)
    {
        return getValueInRangeOrDefault (parameter, nullptr, nullptr, defaultValue);
    }

    template<typename T>
    get_set_t<T> getValueInRangeOrDefault (const ParameterSyntax* parameter,
        const ProtocolParameter* start, const ParameterSyntax* stop, const T& defaultValue)
    {
        ProtocolParameter* par = findInRange (parameter, start, stop, true);
        if (par)
        {
            return par->get<T> ();
        }
        return defaultValue;
    }

private:
    ProtocolParameter* findParameter (const ParameterSyntax* parameter, 
        const ProtocolParameter* start, const ParameterSyntax* stop, bool optional);


private:
    struct ProtocolSyntax *m_syntax;
    std::vector<ProtocolParameter> m_parameters;

#ifdef WITH_UNITTESTS
public:
    static void unitTest ();
#endif
};


#endif
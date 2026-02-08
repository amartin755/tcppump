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

#include "syntax.hpp"
#include "ipaddress.hpp"
#include "macaddress.hpp"
#include "uuid.hpp"

class ProtocolParameter;

class Protocol
{
    struct ProtocolSyntax *m_syntax;
    std::vector<ProtocolParameter> m_parameters;
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
            calcNextRandom8 ();
        return static_cast<uint8_t> (std::get<uint64_t> (m_value));
    }
    uint16_t asInt16 ()
    {
        if (m_isRandom)
            calcNextRandom16 ();
        return static_cast<uint16_t> (std::get<uint64_t> (m_value));
    }
    uint32_t asInt32 ()
    {
        if (m_isRandom)
            calcNextRandom32 ();
        return static_cast<uint32_t> (std::get<uint64_t> (m_value));
    }
    uint64_t asInt64 ()
    {
        if (m_isRandom)
            calcNextRandom64 ();
        return static_cast<uint64_t> (std::get<uint64_t> (m_value));
    }
    double asDouble ()
    {
        if (m_isRandom)
            calcNextRandomDouble ();
        return std::get<double> (m_value);
    }
    const cMacAddress& asMac ()
    {
        if (m_isRandom)
            calcNextRandomMac ();
        return std::get<cMacAddress> (m_value);
    }
    const cIPv4& asIPv4 ()
    {
        if (m_isRandom)
            calcNextRandomIPv4 ();
        return std::get<cIPv4> (m_value);
    }
    const cIPv6& asIPv6 ()
    {
        if (m_isRandom)
            calcNextRandomIPv6 ();
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

    bool isQuotedString () const
    {
        return m_strValueLen >= 2 && *m_strValue == '"' && *(m_strValue + m_strValueLen - 1) == '"';
    }
    void calcNextRandom8 ();
    void calcNextRandom16 ();
    void calcNextRandom32 ();
    void calcNextRandom64 ();
    void calcNextRandomDouble ();
    void calcNextRandomMac ();
    void calcNextRandomIPv4 ();
    void calcNextRandomIPv6 ();
    void calcNextRandomStream ();

    const char* m_name;
    size_t      m_nameLen;
    const char* m_strValue;
    size_t      m_strValueLen;

    const struct ParameterSyntax *m_syntax;
    bool m_isRandom;
    Type m_type; // normalerweise == m_syntax->type, bei mutli types nur der tatsächlich ermittelte Typ

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

    // TODO do we need unique id that it can be part of a map?


};

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


#include <cctype>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <stdexcept>      // std::invalid_argument

#include "parameterlist.hpp"

#include "bug.hpp"
#include "parsehelper.hpp"
#include "random.hpp"



cParameter::cParameter ()
{
    clear ();
}


cParameter::cParameter (const cParameter& obj)
{
    parameter = obj.parameter;
    parLen    = obj.parLen;
    value     = obj.value;
    valLen    = obj.valLen;
    index     = obj.index;
    if (obj.dataLen)
    {
        BUG_ON (!obj.data);
        dataLen   = obj.dataLen;
        data      = new uint8_t[dataLen];
        std::memcpy (data, obj.data, dataLen);
    }
    else
    {
        data    = nullptr;
        dataLen = 0;
    }
}

cParameter::~cParameter ()
{
    delete[] data;
}


void cParameter::clear ()
{
    parameter = NULL;
    parLen    = 0;
    value     = NULL;
    valLen    = 0;
    index     = -1;
    data      = nullptr;
    dataLen   = 0;
}


int cParameter::isRandom (bool allowRange) const
{
    if (!valLen || *value != '*')
        return -1;

    if (valLen == 1)
        return 0;

    if (allowRange)
    {
        char* end;
        errno = 0;
        unsigned long r = strtoul (value+1, &end, 0);
        if (end != (value + valLen))
        {
            throw FormatException (exParFormat, value+1, (int)valLen-1);
        }
        else if ((!((r >= 1) && (r <= 65535))) || (errno == ERANGE))
        {
            throw FormatException (exParRange, value+1, (int)valLen-1);
        }
        return (int)r;
    }
    else
    {
        throw FormatException (exParFormat, value+1, (int)valLen-1);
    }
}


uint32_t cParameter::asInt32 (uint32_t rangeBegin, uint32_t rangeEnd) const
{
    BUG_ON (rangeEnd < rangeBegin);

    unsigned long v;

    if (isRandom(false) == 0)
    {
        uint32_t range = rangeEnd - rangeBegin + 1;
        v = (range ? cRandom::rand32() % range : cRandom::rand32()) + rangeBegin;
    }
    else
    {
        char* end;
        errno = 0;
        v = strtoul (value, &end, 0);
        if (end != (value + valLen))
        {
            throw FormatException (exParFormat, value, (int)valLen);
        }
        else if ((!((v >= rangeBegin) && (v <= rangeEnd))) || (errno == ERANGE))
        {
            throw FormatException (exParRange, value, (int)valLen);
        }
    }

    return (uint32_t)v;
}


uint64_t cParameter::asInt64 (uint64_t rangeBegin, uint64_t rangeEnd) const
{
    BUG_ON (rangeEnd < rangeBegin);

    unsigned long long v;

    if (isRandom(false) == 0)
    {
        uint64_t range = rangeEnd - rangeBegin + 1;
        v = (range ? cRandom::rand64() % range : cRandom::rand64()) + rangeBegin;
    }
    else
    {
        char* end;
        errno = 0;
        v = strtoull (value, &end, 0);
        if (end != (value + valLen))
        {
            throw FormatException (exParFormat, value, (int)valLen);
        }
        else if ((!((v >= rangeBegin) && (v <= rangeEnd))) || (errno == ERANGE))
        {
            throw FormatException (exParRange, value, (int)valLen);
        }
    }

    return (uint64_t)v;
}


uint16_t cParameter::asInt16 (uint16_t rangeBegin, uint16_t rangeEnd) const
{
    return (uint16_t) asInt32 (rangeBegin, rangeEnd);
}


uint8_t cParameter::asInt8 (uint8_t  rangeBegin, uint8_t rangeEnd) const
{
    return (uint8_t) asInt32 (rangeBegin, rangeEnd);
}


double cParameter::asDouble (double rangeBegin, double rangeEnd) const
{
    BUG_ON (rangeEnd < rangeBegin);

    size_t end;
    double v = 0;

    try
    {
        v = std::stod (value, &end);
    }
    catch (const std::out_of_range&)
    {
        throw FormatException (exParRange, value, (int)valLen);
    }
    catch (...)
    {
        throw FormatException (exParFormat, value, (int)valLen);
    }

    if (end != valLen)
    {
        throw FormatException (exParFormat, value, (int)valLen);
    }
    else if ((v < rangeBegin) || (v > rangeEnd))
    {
        throw FormatException (exParRange, value, (int)valLen);
    }

    return v;
}


cMacAddress cParameter::asMac () const
{
    if (isRandom(false) == 0)
    {
        cMacAddress mac (true, false);
        return mac;
    }
    else
    {
        cMacAddress mac;
        if (mac.set(value, valLen))
        {
            return mac;
        }
        else
        {
            throw FormatException(exParFormat, value, (int)valLen);
        }
    }
}


const uint8_t* cParameter::asStream (size_t& len)
{
    bool dummy;
    return asStream (false, dummy, len);
}


const uint8_t* cParameter::asEmbedded (bool &isEmbedded, size_t& len)
{
    return asStream (true, isEmbedded, len);
}


const uint8_t* cParameter::asStream (bool allowEmbPacket, bool &isEmbedded, size_t& len)
{
    if (!data)
    {
        isEmbedded = false;
        int randLen = isRandom (true);

        if (randLen >= 0)
        {
            if (!randLen) randLen = 32; // set to default if no length is defined

            data = (uint8_t*) new uint32_t[randLen / 4 + 1]; // ensure 32bit alignment
            dataLen = randLen;

            cRandom::rand (data, dataLen);
        }
        else if ( (allowEmbPacket && (*value == '"' || *value == '<')) ||
                 (!allowEmbPacket && *value == '"'))
        {
            isEmbedded = *value == '<';
            len = valLen - 2; // don't count " or <> at begin and end of string/embedded packet
            return (uint8_t*)value + 1;
        }
        else
        {
            data = cParseHelper::hexStringToBin(value, valLen, dataLen);
            if (!data)
                throw FormatException (exParFormat, value, (int)valLen);
        }
    }

    len = dataLen;

    return data;
}


cIPv4 cParameter::asIPv4 () const
{
    cIPv4 ip;
    if (isRandom(false) == 0)
    {
        ip.setRandom ();
        return ip;
    }
    else
    {
        if (ip.set(value, valLen))
        {
            return ip;
        }
        else
        {
            throw FormatException (exParFormat, value, (int)valLen);
        }
    }
}


cIPv6 cParameter::asIPv6 () const
{
    cIPv6 ip;
    if (isRandom(false) == 0)
    {
        ip.setRandom ();
        return ip;
    }
    else
    {
        if (ip.set(value, valLen))
        {
            return ip;
        }
        else
        {
            throw FormatException (exParFormat, value, (int)valLen);
        }
    }
}


void cParameter::throwValueException (void) const
{
    throw FormatException (exParFormat, value, (int)valLen);
}


cParameterList::cParameterList (const char* parameters, bool ignoreTrailingGarbage)
{
    parseError = parseParameters (parameters, ignoreTrailingGarbage);
}


bool cParameterList::isValid (void)
{
    return parseError == NULL;
}


const char* cParameterList::getParseError (void)
{
    return parseError;
}


void cParameterList::checkForUnusedParameters (void)
{
    BUG_ON (list.size() != used.size());

    for (size_t n = 0; n < list.size (); n++)
    {
        if (!used.at(n))
            throw FormatException (exParUnused, list[n].parameter, (int)list[n].parLen);
    }
}


//FIXME there's a lot of room for improvement; too many string compares
cParameter* cParameterList::findParameter (const cParameter* startAfter, const char* stopAt, const char* parameter, bool isOptional)
{
    unsigned n = 0;

    if (startAfter)
        n = startAfter->index >= 0 ? startAfter->index + 1 : (unsigned)list.size();
    size_t len = strlen (parameter);
    size_t len2 = stopAt ? strlen (stopAt) : 0;

    for (; n < list.size (); n++)
    {
        cParameter &par = list.at (n);

        if (len2 == par.parLen && !strncmp (stopAt, par.parameter, par.parLen))
        {
            break;
        }
        if (len == par.parLen && !strncmp (parameter, par.parameter, par.parLen))
        {
            used.at(n) = true;
            return &par;
        }
    }

    if (!isOptional)
        throw FormatException (exParUnknown, parameter, (int)len);

    return NULL;
}

cParameter* cParameterList::findParameter (const char* parameter, bool isOptional)
{
    return findParameter (nullptr, nullptr, parameter, isOptional);
}


cParameter* cParameterList::findParameter (const cParameter* startAfter, const char* stopAt, const char* parameter, uint32_t optionalValue)
{
    cParameter* p = findParameter (startAfter, stopAt, parameter, true);
    if (p)
    {
        return p;
    }
    else
    {
        defaultParameter.int32 = optionalValue;
        return &defaultParameter;
    }
}


cParameter* cParameterList::findParameter (const cParameter* startAfter, const char* stopAt, const char* parameter, double optionalValue)
{
    cParameter* p = findParameter (startAfter, stopAt, parameter, true);
    if (p)
    {
        return p;
    }
    else
    {
        defaultParameter.dbl = optionalValue;
        return &defaultParameter;
    }
}


cParameter* cParameterList::findParameter (const cParameter* startAfter, const char* stopAt, const char* parameter, const cMacAddress& optionalValue)
{
    cParameter* p = findParameter (startAfter, stopAt, parameter, true);
    if (p)
    {
        return p;
    }
    else
    {
        defaultParameter.mac = optionalValue;
        return &defaultParameter;
    }
}


cParameter* cParameterList::findParameter (const cParameter* startAfter, const char* stopAt, const char* parameter, const cIPv4& optionalValue)
{
    cParameter* p = findParameter (startAfter, stopAt, parameter, true);
    if (p)
    {
        return p;
    }
    else
    {
        defaultParameter.ip.set(optionalValue);
        return &defaultParameter;
    }
}


cParameter* cParameterList::findParameter (const char* parameter, uint32_t optionalValue)
{
    return findParameter (nullptr, nullptr, parameter, optionalValue);
}


cParameter* cParameterList::findParameter (const char* parameter, double optionalValue)
{
    return findParameter (nullptr, nullptr, parameter, optionalValue);
}


cParameter* cParameterList::findParameter (const char* parameter, const cMacAddress& optionalValue)
{
    return findParameter (nullptr, nullptr, parameter, optionalValue);
}


cParameter* cParameterList::findParameter (const char* parameter, const cIPv4& optionalValue)
{
    return findParameter (nullptr, nullptr, parameter, optionalValue);
}

// returns nullptr on success. Otherwise a pointer to the syntax error
const char* cParameterList::parseParameters (const char* parameters, bool ignoreTrailingGarbage)
{
    static const char BOOLVALUE[] = "1";
    const char* p = parameters;
    cParameter v;
    bool isString = false;
    bool isEmbedded = false;

    list.clear ();
    used.clear ();

    /*
     * Parsing rules
     * --> (par1=value, par2=value, ...) or ( par1 = value , par2 = value , ...)
     *
     * - parameter name starts with letter (digit is not allowed)
     * - letters, digits and _ are allowed for parameter names
     * - parameter name ends with first whitespace or '='
     * - value starts with first non-whitespace after the '='
     * - value ends with first whitespace
     * - next parameter have to be separated via ','
     */

    // caller must ensure that parameter list starts with '('
    BUG_ON (*p != '(');
    p++;

    while (*p != '\0' && *p != ')')
    {
        const char* token;
        v.clear();

        // find start of parameter
        token = cParseHelper::nextKeyStart (p);
        if (!token)
            return p; // syntax error
        else
            p = token;

        // find end of parameter
        p = cParseHelper::nextKeyEnd (p);
        v.parameter = token;
        v.parLen    = p - token;
        p = cParseHelper::skipWhitespaces (p);
        if (!cParseHelper::isOneOf (*p, ",)")) //--> parameter with value
        {
            if (*p != '=')
                return p; // syntax error
            p++;

            // find start of value
            token = cParseHelper::nextValueStart (p);
            if (!token)
                return p; // syntax error
            else
                p = token;
            isString   = *p == '"';
            isEmbedded = *p == '<';

            // find end of value
            if (isString || isEmbedded)
            {
                p++;
                const char* end = std::strchr (p, isString ? '"' : '>');
                if (!end)
                    return p;
                p = *end == '\0' ? end : end + 1;
            }
            else
                p = cParseHelper::nextValueEnd (p);
            v.value  = token;
            v.valLen = p - token;
        }
        else    // --> parameter without value
        {
            v.value  = BOOLVALUE;
            v.valLen = sizeof (BOOLVALUE) - 1;
        }
        // store parameter and its value
        v.index = (int)list.size ();
        list.push_back (v);
        used.push_back(false);

        p = cParseHelper::skipWhitespaces (p);
        if (!cParseHelper::isOneOf (*p, ",)"))
            return p; // syntax error

        if (*p == ',')
            p++;
    }

    if (*p != ')')
        return p;

    if (!ignoreTrailingGarbage)
    {
        p++;
        p = cParseHelper::skipWhitespaces (p);
        if (*p != '\0')
            return p;
    }
    return nullptr; // everything ok
}


#ifdef WITH_UNITTESTS
#include "console.hpp"
void cParameterList::unitTest ()
{
    Console::PrintDebug("-- " __FILE__ " --\n");

    bool catched = false;

    try
    {
        cParameterList obj ("(     first=100, second = 200, third   =300)");
        BUG_IF_NOT (obj.isValid ());
        BUG_IF_NOT (obj.findParameter("first")->asInt32()  == (uint32_t)100);
        BUG_IF_NOT (obj.findParameter("second")->asInt32() == (uint32_t)200);
        BUG_IF_NOT (obj.findParameter("third")->asInt32()  == (uint32_t)300);
    }
    catch (FormatException& )
    {
        BUG_IF_NOT (0);
    }

    {
        cParameterList obj ("(first=100, second = 200, third   =300, fourth=x12)");
        BUG_IF_NOT (obj.isValid ());
        try
        {
            catched = false;
            BUG_IF_NOT (obj.findParameter("first")->asInt32()  == (uint32_t)100);
            BUG_IF_NOT (obj.findParameter("second")->asInt32() == (uint32_t)200);
            BUG_IF_NOT (obj.findParameter("third")->asInt32()  == (uint32_t)300);
        }
        catch (FormatException& )
        {
            BUG_IF_NOT (0);
        }

        try
        {
            catched = false;
            obj.findParameter("third")->asMac();
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_IF_NOT (e.what () == exParFormat);
            BUG_IF_NOT (e.value ());
        }
        BUG_IF_NOT (catched);

        try
        {
            catched = false;
            obj.findParameter("fourth")->asIPv4();
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_IF_NOT (e.what () == exParFormat);
            BUG_IF_NOT (e.value ());
        }
        BUG_IF_NOT (catched);

        try
        {
            catched = false;
            obj.findParameter("f");
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_IF_NOT (e.what () == exParUnknown);
            BUG_IF_NOT (e.value ());
            BUG_IF_NOT (!strcmp ("f", e.value ()));
        }
        BUG_IF_NOT (catched);

        try
        {
            catched = false;
            obj.findParameter("firstfirst");
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_IF_NOT (e.what () == exParUnknown);
            BUG_IF_NOT (e.value ());
            BUG_IF_NOT (!strcmp ("firstfirst", e.value ()));
        }
        BUG_IF_NOT (catched);

        try
        {
            BUG_IF_NOT (obj.findParameter("first")->asInt32(100, 100)  == (uint32_t)100);
        }
        catch (FormatException& )
        {
            BUG_IF_NOT (0);
        }

        try
        {
            catched = false;
            obj.findParameter("first")->asInt32(101, 102);
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_IF_NOT (e.what () == exParRange);
            BUG_IF_NOT (e.value ());
        }
        BUG_IF_NOT (catched);

    }

    try
    {
        cParameterList obj ("(first=100, firstsecond = 200, third   =300)");
        BUG_IF_NOT (obj.findParameter("first")->asInt32()  == (uint32_t)100);
        BUG_IF_NOT (obj.findParameter("firstsecond")->asInt32()  == (uint32_t)200);
        BUG_IF_NOT (obj.isValid ());
    }
    catch (FormatException& )
    {
        BUG_IF_NOT (0);
    }

    {
        cParameterList obj ("(first=100.firstsecond = 200, third   =300)");
        BUG_IF_NOT (!obj.isValid ());
    }
    {
        cParameterList obj ("(d.first=100.second =)");
        BUG_IF_NOT (!obj.isValid ());
    }
    {
        cParameterList obj ("(=123)");
        BUG_IF_NOT (!obj.isValid ());
    }
    try
    {
        cParameterList obj ("(  first=123)");
        BUG_IF_NOT (obj.isValid ());
        BUG_IF_NOT (obj.findParameter("first")->asInt32()  == (uint32_t)123);
    }
    catch (FormatException& )
    {
        BUG_IF_NOT (0);
    }
    {
        cParameterList obj ("($,dkfjsdf=sd,djhdslk,,0=0sd sdlfkjf)");
        BUG_IF_NOT (!obj.isValid ());
    }
    {
        cParameterList obj ("(long=100, ipv4 = 1.2.3.4, mac =12:34:56:78:9A:BC, payload=012345)");
        try
        {
            BUG_IF_NOT (obj.isValid ());
            BUG_IF_NOT (obj.findParameter("long")->asInt32()  == (uint32_t)100);
            BUG_IF_NOT (obj.findParameter("ipv4")->asIPv4()  == cIPv4("1.2.3.4"));
            cMacAddress mac2("12:34:56:78:9a:bc");
            cMacAddress mac = obj.findParameter("mac")->asMac();
            BUG_IF_NOT (!memcmp (&mac, &mac2, sizeof (mac2)));
            size_t len = 0;
            BUG_IF_NOT (!memcmp (obj.findParameter("payload")->asStream(len), "\x01\x23\x45", len));
            BUG_IF_NOT (len == 3);
        }
        catch (FormatException& )
        {
            BUG_IF_NOT (0);
        }
    }
    {
        cParameterList obj ("(ipv4 = 1.2.3.4)");
        try
        {
            BUG_IF_NOT (obj.isValid ());
            BUG_IF_NOT (obj.findParameter("ipv4")->asIPv4()  == cIPv4("1.2.3.4"));
            cMacAddress mac1("12:34:56:78:9a:bc");
            cMacAddress mac2("11:22:33:44:55:66");
            cMacAddress _mac2 = obj.findParameter("mac2", mac2)->asMac();
            cMacAddress _mac1 = obj.findParameter("mac1", mac1)->asMac();
            BUG_IF_NOT (!memcmp (&_mac2, &mac2, sizeof (mac2)));
            BUG_IF_NOT (!memcmp (&_mac1, &mac1, sizeof (mac2)));
        }
        catch (FormatException& )
        {
            BUG_IF_NOT (0);
        }
    }
    {
        cParameterList obj ("(first=0xFFFF, second=0x10000, toolong=0x100000000)");
        BUG_IF_NOT (obj.isValid ());
        try
        {
            BUG_IF_NOT (obj.findParameter("first")->asInt32()  == (uint32_t)0x0000ffff);
            BUG_IF_NOT (obj.findParameter("first")->asInt32(200, 0xffff)  == (uint32_t)0x0000ffff);
            BUG_IF_NOT (obj.findParameter("first")->asInt32(0xffff, 0xffff)  == (uint32_t)0x0000ffff);
            BUG_IF_NOT (obj.findParameter("first")->asInt16()  == (uint16_t)0x0000ffff);
            BUG_IF_NOT (obj.findParameter("second")->asInt32()  == (uint32_t)0x00010000);
            BUG_IF_NOT (obj.findParameter("second")->asInt32(0x00010000)  == (uint32_t)0x00010000);
        }
        catch (FormatException& )
        {
            BUG_IF_NOT (0);
        }
        try
        {
            catched = false;
            obj.findParameter("first")->asInt8();
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_IF_NOT (e.what () == exParRange);
            BUG_IF_NOT (e.value ());
        }
        BUG_IF_NOT (catched);
        try
        {
            catched = false;
            obj.findParameter("second")->asInt16();
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_IF_NOT (e.what () == exParRange);
            BUG_IF_NOT (e.value ());
        }
        BUG_IF_NOT (catched);
        try
        {
            catched = false;
            obj.findParameter("toolong")->asInt32();
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_IF_NOT (e.what () == exParRange);
            BUG_IF_NOT (e.value ());
        }
        BUG_IF_NOT (catched);
        {
            cParameterList obj ("(dk=,fjsdf=12)");
            BUG_IF_NOT (!obj.isValid ());
        }
    }
    {
        cParameterList obj ("(first=0x1, first=0x2, first=0x3 )");
        BUG_IF_NOT (obj.isValid ());
        try
        {
            const cParameter* par = nullptr;
            BUG_IF_NOT (obj.findParameter("first")->asInt32()  == (uint32_t)1);

            par = obj.findParameter("first");
            BUG_IF_NOT (par->asInt32()  == (uint32_t)1);
            par = obj.findParameter(par, nullptr, "first");
            BUG_IF_NOT (par->asInt32()  == (uint32_t)2);
            par = obj.findParameter(par, nullptr, "first");
            BUG_IF_NOT (par->asInt32()  == (uint32_t)3);
            par = obj.findParameter(par, nullptr, "first", true);
            BUG_IF_NOT (!par);
        }
        catch (FormatException& )
        {
            BUG_IF_NOT (0);
        }
    }
    {
        cParameterList obj ("(second=0x10, first=0x1, second=0x10, first=0x2, second=0x20, first=0x3, second=0x30)");
        BUG_IF_NOT (obj.isValid ());
        try
        {
            const cParameter* par = nullptr;
            BUG_IF_NOT (obj.findParameter("first")->asInt32()  == (uint32_t)1);

            par = obj.findParameter("first");
            BUG_IF_NOT (par->asInt32()  == (uint32_t)1);
            BUG_IF_NOT (obj.findParameter(par, nullptr, "second")->asInt32()  == (uint32_t)0x10);

            par = obj.findParameter(par, nullptr, "first");
            BUG_IF_NOT (par->asInt32()  == (uint32_t)2);
            BUG_IF_NOT (obj.findParameter(par, nullptr, "second")->asInt32()  == (uint32_t)0x20);

            par = obj.findParameter(par, nullptr, "first");
            BUG_IF_NOT (par->asInt32()  == (uint32_t)3);
            BUG_IF_NOT (obj.findParameter(par, nullptr, "second")->asInt32()  == (uint32_t)0x30);

            par = obj.findParameter(par, nullptr, "first", true);
            BUG_IF_NOT (!par);
        }
        catch (FormatException& )
        {
            BUG_IF_NOT (0);
        }
    }
    {
        cParameterList obj ("(second=0x10, first=0x1, first=0x2, second=0x20, first=0x3, second=0x30)");
        BUG_IF_NOT (obj.isValid ());
        try
        {
            const cParameter* par = nullptr;

            par = obj.findParameter("first");
            BUG_IF_NOT (par->asInt32()  == (uint32_t)1);
            BUG_IF_NOT (!obj.findParameter(par, "first", "second", true));

            par = obj.findParameter(par, nullptr, "first");
            BUG_IF_NOT (par->asInt32()  == (uint32_t)2);
            BUG_IF_NOT (obj.findParameter(par, "first", "second")->asInt32()  == (uint32_t)0x20);

            par = obj.findParameter(par, nullptr, "first");
            BUG_IF_NOT (par->asInt32()  == (uint32_t)3);
            BUG_IF_NOT (obj.findParameter(par, nullptr, "second")->asInt32()  == (uint32_t)0x30);

            par = obj.findParameter(par, nullptr, "first", true);
            BUG_IF_NOT (!par);
        }
        catch (FormatException& )
        {
            BUG_IF_NOT (0);
        }
    }
    {
        cParameterList obj ("(good=0.1, good2=1, good3=1.0e3, bad=abcd, bad2=3.4., bad3=1.0e400)");
        BUG_IF_NOT (obj.isValid ());
        try
        {
            BUG_IF_NOT (obj.findParameter("good")->asDouble()  == 0.1);
        }
        catch (...)
        {
            BUG_IF_NOT (0);
        }
        try
        {
            BUG_IF_NOT (obj.findParameter("good2")->asDouble() == 1.0);
        }
        catch (...)
        {
            BUG_IF_NOT (0);
        }
        try
        {
            BUG_IF_NOT (obj.findParameter("good3")->asDouble() == 1000.0);
        }
        catch (...)
        {
            BUG_IF_NOT (0);
        }
        catched = false;
        try
        {
            obj.findParameter("bad")->asDouble();
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_IF_NOT (e.what () == exParFormat);
            BUG_IF_NOT (e.value ());
        }
        BUG_IF_NOT (catched);
        catched = false;
        try
        {
            obj.findParameter("bad2")->asDouble();
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_IF_NOT (e.what () == exParFormat);
            BUG_IF_NOT (e.value ());
        }
        BUG_IF_NOT (catched);
        try
        {
            obj.findParameter("bad3")->asDouble();
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_IF_NOT (e.what () == exParRange);
            BUG_IF_NOT (e.value ());
        }
        BUG_IF_NOT (catched);
    }

    try
    {
        size_t len = 0;
        cParameterList obj ("(first=\"Hello World\", second = \"\", third   =300)");
        BUG_IF_NOT (obj.isValid ());
        BUG_IF_NOT (!std::memcmp (obj.findParameter("first")->asStream(len), "Hello World", len));
        obj.findParameter("second")->asStream(len);
        BUG_IF_NOT (len == 0);
        BUG_IF_NOT (obj.findParameter("third")->asInt32()  == (uint32_t)300);
    }
    catch (FormatException& )
    {
        BUG_IF_NOT (0);
    }
    try
    {
        cParameterList obj ("(first=\"Hello World, second = 2, third   =300)");
        BUG_IF_NOT (!obj.isValid ());
    }
    catch (FormatException& )
    {
        BUG_IF_NOT (0);
    }
    {
        cParameterList obj ("(first=*)");
        BUG_IF_NOT (obj.isValid ());
        try
        {
            const cParameter* par = nullptr;

            par = obj.findParameter("first");
            BUG_IF_NOT (par->asInt32(0,4)  < (uint32_t)5);
            BUG_IF_NOT (par->asInt32(0,4)  < (uint32_t)5);
            BUG_IF_NOT (par->asInt32(0,4)  < (uint32_t)5);
            BUG_IF_NOT (par->asInt32(0,4)  < (uint32_t)5);
        }
        catch (FormatException& )
        {
            BUG_IF_NOT (0);
        }
    }
    {
        catched = false;
        cParameterList obj ("(first=*k)");
        BUG_IF_NOT (obj.isValid ());
        try
        {
            const cParameter* par = nullptr;
            par = obj.findParameter("first");
            BUG_IF_NOT (par);
            par->asInt32();
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_IF_NOT (e.what () == exParFormat);
            BUG_IF_NOT (e.value ());
        }
        BUG_IF_NOT (catched);
    }
    {
        catched = false;
        cParameterList obj ("(first=*1k)");
        BUG_IF_NOT (obj.isValid ());
        try
        {
            const cParameter* par = nullptr;
            par = obj.findParameter("first");
            BUG_IF_NOT (par);
            par->asInt32();
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_IF_NOT (e.what () == exParFormat);
            BUG_IF_NOT (e.value ());
        }
        BUG_IF_NOT (catched);
    }
    {
        catched = false;
        cParameterList obj ("(first=**)");
        BUG_IF_NOT (obj.isValid ());
        try
        {
            const cParameter* par = nullptr;
            par = obj.findParameter("first");
            BUG_IF_NOT (par);
            par->asInt32();
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_IF_NOT (e.what () == exParFormat);
            BUG_IF_NOT (e.value ());
        }
        BUG_IF_NOT (catched);
    }
    {
        catched = false;
        cParameterList obj ("(first=*1)");
        BUG_IF_NOT (obj.isValid ());
        try
        {
            const cParameter* par = nullptr;
            par = obj.findParameter("first");
            BUG_IF_NOT (par);
            par->asInt32();
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_IF_NOT (e.what () == exParFormat);
            BUG_IF_NOT (e.value ());
        }
        BUG_IF_NOT (catched);
    }
    {
        catched = false;
        cParameterList obj ("(first=*k)");
        BUG_IF_NOT (obj.isValid ());
        try
        {
            size_t len = 0;
            cParameter* par = nullptr;
            par = obj.findParameter("first");
            BUG_IF_NOT (par);
            par->asStream(len);
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_IF_NOT (e.what () == exParFormat);
            BUG_IF_NOT (e.value ());
        }
        BUG_IF_NOT (catched);
    }
    {
        catched = false;
        cParameterList obj ("(first=*1k)");
        BUG_IF_NOT (obj.isValid ());
        try
        {
            size_t len = 0;
            cParameter* par = nullptr;
            par = obj.findParameter("first");
            BUG_IF_NOT (par);
            par->asStream(len);
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_IF_NOT (e.what () == exParFormat);
            BUG_IF_NOT (e.value ());
        }
        BUG_IF_NOT (catched);
    }
    {
        catched = false;
        cParameterList obj ("(first=**)");
        BUG_IF_NOT (obj.isValid ());
        try
        {
            size_t len = 0;
            cParameter* par = nullptr;
            par = obj.findParameter("first");
            BUG_IF_NOT (par);
            par->asStream(len);
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_IF_NOT (e.what () == exParFormat);
            BUG_IF_NOT (e.value ());
        }
        BUG_IF_NOT (catched);
    }
    {
        catched = false;
        cParameterList obj ("(first=*65536)");
        BUG_IF_NOT (obj.isValid ());
        try
        {
            size_t len = 0;
            cParameter* par = nullptr;
            par = obj.findParameter("first");
            BUG_IF_NOT (par);
            par->asStream(len);
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_IF_NOT (e.what () == exParRange);
            BUG_IF_NOT (e.value ());
        }
        BUG_IF_NOT (catched);
    }
    {
        catched = false;
        cParameterList obj ("(first=*0)");
        BUG_IF_NOT (obj.isValid ());
        try
        {
            size_t len = 0;
            cParameter* par = nullptr;
            par = obj.findParameter("first");
            BUG_IF_NOT (par);
            par->asStream(len);
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_IF_NOT (e.what () == exParRange);
            BUG_IF_NOT (e.value ());
        }
        BUG_IF_NOT (catched);
    }
    {
        cParameterList obj ("(first=*, second=*1, third=*100)");
        try
        {
            size_t len = 0;
            BUG_IF_NOT (obj.isValid ());
            len = 0;
            BUG_IF_NOT (obj.findParameter("first")->asStream(len));
            BUG_IF_NOT (len == 32);
            len = 0;
            BUG_IF_NOT (obj.findParameter("second")->asStream(len));
            BUG_IF_NOT (len == 1);
            len = 0;
            BUG_IF_NOT (obj.findParameter("third")->asStream(len));
            BUG_IF_NOT (len == 100);
        }
        catch (FormatException& )
        {
            BUG_IF_NOT (0);
        }
    }
    {
        try
        {
            cParameterList obj ("(first = 01)3456789abcdef0123456789abcdef)");
            BUG_IF_NOT (!obj.isValid ());
        }
        catch (FormatException& )
        {
            BUG_IF_NOT (0);
        }
    }
    try
    {
        cParameterList obj ("(     first, second = 200, third)");
        BUG_IF_NOT (obj.isValid ());
        BUG_IF_NOT (obj.findParameter("first")->asInt32()  == (uint32_t)1);
        BUG_IF_NOT (obj.findParameter("second")->asInt32() == (uint32_t)200);
        BUG_IF_NOT (obj.findParameter("third")->asInt32()  == (uint32_t)1);
    }
    catch (FormatException& )
    {
        BUG_IF_NOT (0);
    }
    try
    {
        cParameterList obj ("(     first=100, second , third   )");
        BUG_IF_NOT (obj.isValid ());
        BUG_IF_NOT (obj.findParameter("first")->asInt32()  == (uint32_t)100);
        BUG_IF_NOT (obj.findParameter("second")->asInt32() == (uint32_t)1);
        BUG_IF_NOT (obj.findParameter("third")->asInt32()  == (uint32_t)1);
    }
    catch (FormatException& )
    {
        BUG_IF_NOT (0);
    }

    try
    {
        cParameterList obj ("(     first=100, second = 200, third   =300)");
        BUG_IF_NOT (obj.isValid ());
        int n = 0;
        for (auto &parameter : obj)
        {
            auto name = parameter.name();
            switch (n++)
            {
            case 0:
                BUG_ON (std::strncmp (name.first, "first", name.second));
                BUG_ON (parameter.asInt32()  != (uint32_t)100);
                break;
            case 1:
                BUG_ON (std::strncmp (name.first, "second", name.second));
                BUG_ON (parameter.asInt32()  != (uint32_t)200);
                break;
            case 2:
                BUG_ON (std::strncmp (name.first, "third", name.second));
                BUG_ON (parameter.asInt32()  != (uint32_t)300);
                break;
            }
        }
        BUG_ON (n != 3);
    }
    catch (FormatException& )
    {
        BUG ("unexpected exception");
    }
    try
    {
        cParameterList obj ("(     first=100, second = 200, third   =300)");
        BUG_IF_NOT (obj.isValid ());
        int n = 0;
        for (const auto &parameter : obj)
        {
            auto name = parameter.name();
            switch (n++)
            {
            case 0:
                BUG_ON (std::strncmp (name.first, "first", name.second));
                BUG_ON (parameter.asInt32()  != (uint32_t)100);
                break;
            case 1:
                BUG_ON (std::strncmp (name.first, "second", name.second));
                BUG_ON (parameter.asInt32()  != (uint32_t)200);
                break;
            case 2:
                BUG_ON (std::strncmp (name.first, "third", name.second));
                BUG_ON (parameter.asInt32()  != (uint32_t)300);
                break;
            }
        }
        BUG_ON (n != 3);
    }
    catch (FormatException& )
    {
        BUG ("unexpected exception");
    }

    // TODO fuzzing
}
#endif

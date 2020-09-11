/**
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2020 Andreas Martin (netnag@mailbox.org)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
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

#include "bugon.h"
#include "parameterlist.hpp"
#include "parsehelper.hpp"
#include "converter.hpp"



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
        BUG_ON (obj.data);
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


uint32_t cParameter::asInt32 (uint32_t rangeBegin, uint32_t rangeEnd) const
{
    BUG_ON (rangeEnd >= rangeBegin);

    unsigned long v;

    char* end;
    errno = 0;
    v = strtoul (value, &end, 0);
    if (end != (value + valLen))
    {
        throw FormatException (exParFormat, value);
    }
    else if ((!((v >= rangeBegin) && (v <= rangeEnd))) || (errno == ERANGE))
    {
        throw FormatException (exParRange, value);
    }

    return (uint32_t)v;
}


uint16_t cParameter::asInt16 (uint16_t rangeBegin, uint16_t rangeEnd) const
{
    return (uint16_t) asInt32 (rangeBegin, rangeEnd);
}


uint8_t  cParameter::asInt8 (uint8_t  rangeBegin, uint8_t rangeEnd) const
{
    return (uint8_t) asInt32 (rangeBegin, rangeEnd);
}


cMacAddress cParameter::asMac () const
{
    cMacAddress mac;

    if (mac.set(value, valLen))
    {
        return mac;
    }
    else
    {
        throw FormatException(exParFormat, value);
    }
}


const uint8_t* cParameter::asStream (size_t& len)
{
    if (!data)
    {
        data = cParseHelper::hexStringToBin(value, valLen, dataLen);
        if (!data)
            throw FormatException (exParFormat, value);
    }

    len = dataLen;

    return data;
}


cIpAddress cParameter::asIPv4 () const
{
    cIpAddress ip;
    if (ip.set(value, valLen))
    {
        return ip;
    }
    else
    {
        throw FormatException (exParFormat, value);
    }
}


cParameterList::cParameterList (const char* parameters)
{
    parseError = parseParameters (parameters);
}


bool cParameterList::isValid ()
{
    return parseError == NULL;
}


const char* cParameterList::getParseError ()
{
    return parseError;
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
            return &par;
        }
    }

    if (!isOptional)
        throw FormatException (exParUnknown, parameter);

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


cParameter* cParameterList::findParameter (const cParameter* startAfter, const char* stopAt, const char* parameter, const cIpAddress& optionalValue)
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


cParameter* cParameterList::findParameter (const char* parameter, const cMacAddress& optionalValue)
{
    return findParameter (nullptr, nullptr, parameter, optionalValue);
}


cParameter* cParameterList::findParameter (const char* parameter, const cIpAddress& optionalValue)
{
    return findParameter (nullptr, nullptr, parameter, optionalValue);
}

// returns nullptr on success. Otherwise a pointer to the syntax error
const char* cParameterList::parseParameters (const char* parameters)
{
    const char* p = parameters;
    cParameter v;

    list.clear ();

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
    BUG_ON (*p == '(');
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
        if (*p != '=')
            return p; // syntax error
        p++;

        // find start of value
        token = cParseHelper::nextValueStart (p);
        if (!token)
            return p; // syntax error
        else
            p = token;

        // find end of value
        p = cParseHelper::nextValueEnd (p);
        v.value  = token;
        v.valLen = p - token;

        // store parameter and its value
        v.index = (int)list.size ();
        list.push_back (v);

        p = cParseHelper::skipWhitespaces (p);
        if (!cParseHelper::isOneOf (*p, ",)"))
            return p; // syntax error

        if (*p == ',')
            p++;
    }

    return *p == ')' ? nullptr : p;
}


#ifdef WITH_UNITTESTS
#include "console.hpp"
void cParameterList::unitTest ()
{
    nn::Console::PrintDebug("-- " __FILE__ " --\n");

    bool catched = false;

    try
    {
        cParameterList obj ("(     first=100, second = 200, third   =300)");
        BUG_ON (obj.isValid ());
        BUG_ON (obj.findParameter("first")->asInt32()  == (uint32_t)100);
        BUG_ON (obj.findParameter("second")->asInt32() == (uint32_t)200);
        BUG_ON (obj.findParameter("third")->asInt32()  == (uint32_t)300);
    }
    catch (FormatException& )
    {
        BUG_ON (0);
    }

    {
        cParameterList obj ("(first=100, second = 200, third   =300, fourth=x12)");
        BUG_ON (obj.isValid ());
        try
        {
            catched = false;
            BUG_ON (obj.findParameter("first")->asInt32()  == (uint32_t)100);
            BUG_ON (obj.findParameter("second")->asInt32() == (uint32_t)200);
            BUG_ON (obj.findParameter("third")->asInt32()  == (uint32_t)300);
        }
        catch (FormatException& )
        {
            BUG_ON (0);
        }

        try
        {
            catched = false;
            obj.findParameter("third")->asMac();
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_ON (e.what () == exParFormat);
            BUG_ON (e.value ());
        }
        BUG_ON (catched);

        try
        {
            catched = false;
            obj.findParameter("fourth")->asIPv4();
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_ON (e.what () == exParFormat);
            BUG_ON (e.value ());
        }
        BUG_ON (catched);

        try
        {
            catched = false;
            obj.findParameter("f");
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_ON (e.what () == exParUnknown);
            BUG_ON (e.value ());
            BUG_ON (!strcmp ("f", e.value ()));
        }
        BUG_ON (catched);

        try
        {
            catched = false;
            obj.findParameter("firstfirst");
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_ON (e.what () == exParUnknown);
            BUG_ON (e.value ());
            BUG_ON (!strcmp ("firstfirst", e.value ()));
        }
        BUG_ON (catched);

        try
        {
            BUG_ON (obj.findParameter("first")->asInt32(100, 100)  == (uint32_t)100);
        }
        catch (FormatException& )
        {
            BUG_ON (0);
        }

        try
        {
            catched = false;
            obj.findParameter("first")->asInt32(101, 102);
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_ON (e.what () == exParRange);
            BUG_ON (e.value ());
        }
        BUG_ON (catched);

    }

    try
    {
        cParameterList obj ("(first=100, firstsecond = 200, third   =300)");
        BUG_ON (obj.findParameter("first")->asInt32()  == (uint32_t)100);
        BUG_ON (obj.findParameter("firstsecond")->asInt32()  == (uint32_t)200);
        BUG_ON (obj.isValid ());
    }
    catch (FormatException& )
    {
        BUG_ON (0);
    }

    {
        cParameterList obj ("(first=100.firstsecond = 200, third   =300)");
        BUG_ON (!obj.isValid ());
    }
    {
        cParameterList obj ("(d.first=100.second =)");
        BUG_ON (!obj.isValid ());
    }
    {
        cParameterList obj ("(d,first=100)");
        BUG_ON (!obj.isValid ());
    }
    {
        cParameterList obj ("(=123)");
        BUG_ON (!obj.isValid ());
    }
    try
    {
        cParameterList obj ("(  first=123)");
        BUG_ON (obj.isValid ());
        BUG_ON (obj.findParameter("first")->asInt32()  == (uint32_t)123);
    }
    catch (FormatException& )
    {
        BUG_ON (0);
    }
    {
        cParameterList obj ("($,dkfjsdf=sd,djhdslk,,0=0sd sdlfkjf)");
        BUG_ON (!obj.isValid ());
    }
    {
        cParameterList obj ("(long=100, ipv4 = 1.2.3.4, mac =12:34:56:78:9A:BC, payload=012345)");
        try
        {
            BUG_ON (obj.isValid ());
            BUG_ON (obj.findParameter("long")->asInt32()  == (uint32_t)100);
            BUG_ON (obj.findParameter("ipv4")->asIPv4()  == cIpAddress("1.2.3.4"));
            cMacAddress mac2("12:34:56:78:9a:bc");
            cMacAddress mac = obj.findParameter("mac")->asMac();
            BUG_ON (!memcmp (&mac, &mac2, sizeof (mac2)));
            size_t len = 0;
            BUG_ON (!memcmp (obj.findParameter("payload")->asStream(len), "\x01\x23\x45", len));
            BUG_ON (len == 3);
        }
        catch (FormatException& )
        {
            BUG_ON (0);
        }
    }
    {
        cParameterList obj ("(ipv4 = 1.2.3.4)");
        try
        {
            BUG_ON (obj.isValid ());
            BUG_ON (obj.findParameter("ipv4")->asIPv4()  == cIpAddress("1.2.3.4"));
            cMacAddress mac1("12:34:56:78:9a:bc");
            cMacAddress mac2("11:22:33:44:55:66");
            cMacAddress _mac2 = obj.findParameter("mac2", mac2)->asMac();
            cMacAddress _mac1 = obj.findParameter("mac1", mac1)->asMac();
            BUG_ON (!memcmp (&_mac2, &mac2, sizeof (mac2)));
            BUG_ON (!memcmp (&_mac1, &mac1, sizeof (mac2)));
        }
        catch (FormatException& )
        {
            BUG_ON (0);
        }
    }
    {
        cParameterList obj ("(first=0xFFFF, second=0x10000, toolong=0x100000000)");
        BUG_ON (obj.isValid ());
        try
        {
            BUG_ON (obj.findParameter("first")->asInt32()  == (uint32_t)0x0000ffff);
            BUG_ON (obj.findParameter("first")->asInt32(200, 0xffff)  == (uint32_t)0x0000ffff);
            BUG_ON (obj.findParameter("first")->asInt32(0xffff, 0xffff)  == (uint32_t)0x0000ffff);
            BUG_ON (obj.findParameter("first")->asInt16()  == (uint16_t)0x0000ffff);
            BUG_ON (obj.findParameter("second")->asInt32()  == (uint32_t)0x00010000);
            BUG_ON (obj.findParameter("second")->asInt32(0x00010000)  == (uint32_t)0x00010000);
        }
        catch (FormatException& )
        {
            BUG_ON (0);
        }
        try
        {
            catched = false;
            obj.findParameter("first")->asInt8();
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_ON (e.what () == exParRange);
            BUG_ON (e.value ());
        }
        BUG_ON (catched);
        try
        {
            catched = false;
            obj.findParameter("second")->asInt16();
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_ON (e.what () == exParRange);
            BUG_ON (e.value ());
        }
        BUG_ON (catched);
        try
        {
            catched = false;
            obj.findParameter("toolong")->asInt32();
        }
        catch (FormatException& e)
        {
            catched = true;
            BUG_ON (e.what () == exParRange);
            BUG_ON (e.value ());
        }
        BUG_ON (catched);
        {
            cParameterList obj ("(dk,fjsdf=12)");
            BUG_ON (!obj.isValid ());
        }
    }
    {
        cParameterList obj ("(first=0x1, first=0x2, first=0x3 )");
        BUG_ON (obj.isValid ());
        try
        {
            const cParameter* par = nullptr;
            BUG_ON (obj.findParameter("first")->asInt32()  == (uint32_t)1);

            par = obj.findParameter("first");
            BUG_ON (par->asInt32()  == (uint32_t)1);
            par = obj.findParameter(par, nullptr, "first");
            BUG_ON (par->asInt32()  == (uint32_t)2);
            par = obj.findParameter(par, nullptr, "first");
            BUG_ON (par->asInt32()  == (uint32_t)3);
            par = obj.findParameter(par, nullptr, "first", true);
            BUG_ON (!par);
        }
        catch (FormatException& )
        {
            BUG_ON (0);
        }
    }
    {
        cParameterList obj ("(second=0x10, first=0x1, second=0x10, first=0x2, second=0x20, first=0x3, second=0x30)");
        BUG_ON (obj.isValid ());
        try
        {
            const cParameter* par = nullptr;
            BUG_ON (obj.findParameter("first")->asInt32()  == (uint32_t)1);

            par = obj.findParameter("first");
            BUG_ON (par->asInt32()  == (uint32_t)1);
            BUG_ON (obj.findParameter(par, nullptr, "second")->asInt32()  == (uint32_t)0x10);

            par = obj.findParameter(par, nullptr, "first");
            BUG_ON (par->asInt32()  == (uint32_t)2);
            BUG_ON (obj.findParameter(par, nullptr, "second")->asInt32()  == (uint32_t)0x20);

            par = obj.findParameter(par, nullptr, "first");
            BUG_ON (par->asInt32()  == (uint32_t)3);
            BUG_ON (obj.findParameter(par, nullptr, "second")->asInt32()  == (uint32_t)0x30);

            par = obj.findParameter(par, nullptr, "first", true);
            BUG_ON (!par);
        }
        catch (FormatException& )
        {
            BUG_ON (0);
        }
    }
    {
        cParameterList obj ("(second=0x10, first=0x1, first=0x2, second=0x20, first=0x3, second=0x30)");
        BUG_ON (obj.isValid ());
        try
        {
            const cParameter* par = nullptr;

            par = obj.findParameter("first");
            BUG_ON (par->asInt32()  == (uint32_t)1);
            BUG_ON (!obj.findParameter(par, "first", "second", true));

            par = obj.findParameter(par, nullptr, "first");
            BUG_ON (par->asInt32()  == (uint32_t)2);
            BUG_ON (obj.findParameter(par, "first", "second")->asInt32()  == (uint32_t)0x20);

            par = obj.findParameter(par, nullptr, "first");
            BUG_ON (par->asInt32()  == (uint32_t)3);
            BUG_ON (obj.findParameter(par, nullptr, "second")->asInt32()  == (uint32_t)0x30);

            par = obj.findParameter(par, nullptr, "first", true);
            BUG_ON (!par);
        }
        catch (FormatException& )
        {
            BUG_ON (0);
        }
    }

    // TODO fuzzing
}
#endif

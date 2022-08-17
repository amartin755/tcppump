// SPDX-License-Identifier: GPL-3.0-only
/*
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2021 Andreas Martin (netnag@mailbox.org)
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


#ifndef PARAMETERLIST_HPP_
#define PARAMETERLIST_HPP_

#include <vector>
#include <cfloat>

#include "ipaddress.hpp"
#include "macaddress.hpp"
#include "formatexception.hpp"
#include "bug.hpp"

class cParameter
{
public:
    cParameter ();
    cParameter (const cParameter&);
    virtual ~cParameter();

    virtual uint32_t    asInt32 (uint32_t rangeBegin = 0, uint32_t rangeEnd = 0xffffffff) const;
    virtual uint16_t    asInt16 (uint16_t rangeBegin = 0, uint16_t rangeEnd = 0xffff) const;
    virtual uint8_t     asInt8  (uint8_t  rangeBegin = 0, uint8_t rangeEnd = 0xff) const;
    virtual double      asDouble(double rangeBegin = DBL_MIN, double rangeEnd = DBL_MAX) const;
    virtual cMacAddress asMac   () const;
    virtual const uint8_t* asStream (size_t& len);
    virtual const uint8_t* asEmbedded (bool &isEmbedded, size_t& len);
    virtual cIpAddress  asIPv4  () const;

    void throwValueException (void) const;

private:
    void clear ();
    int isRandom (bool allowRange) const;
    const uint8_t* asStream (bool allowEmbPacket, bool &isEmbedded, size_t& len);

    const char* parameter;
    size_t      parLen;
    const char* value;
    size_t      valLen;
    int         index;
    uint8_t*    data;
    size_t      dataLen;

    friend class cParameterList;
};


class cDefaultParameter : public cParameter
{
    friend class cParameterList;

public:
    virtual uint32_t    asInt32 (uint32_t, uint32_t) const {return int32;}
    virtual uint16_t    asInt16 (uint16_t, uint16_t) const {return (uint16_t)int32;}
    virtual uint8_t     asInt8  (uint8_t,  uint8_t) const {return (uint8_t)int32;}
    virtual double      asDouble(double,  double) const {return dbl;}
    virtual cMacAddress asMac   () const {return mac;}
    virtual const uint8_t* asStream   (size_t&)
    {
        BUG ("no raw access for optional parameters");
        return NULL;
    }
    virtual const uint8_t* asEmbedded (bool&, size_t&)
    {
        BUG ("no raw access for optional parameters");
        return NULL;
    }
    virtual cIpAddress  asIPv4  () const {return ip;}

private:
    uint32_t    int32;
    cMacAddress mac;
    cIpAddress  ip;
    double      dbl;
};


class cParameterList
{
public:
    cParameterList (const char*, bool ignoreTrailingGarbage = false);
    bool isValid (void);
    const char* getParseError (void);
    void checkForUnusedParameters (void);
    cParameter* findParameter (const cParameter* startAfter, const char* stopAt, const char* parameter, bool isOptional = false);
    cParameter* findParameter (const char* parameter, bool isOptional = false);
    cParameter* findParameter (const char* parameter, uint32_t optionalValue);
    cParameter* findParameter (const char* parameter, double optionalValue);
    cParameter* findParameter (const char* parameter, const cMacAddress& optionalValue);
    cParameter* findParameter (const char* parameter, const cIpAddress& optionalValue);
    cParameter* findParameter (const cParameter* startAfter, const char* stopAt, const char* parameter, uint32_t optionalValue);
    cParameter* findParameter (const cParameter* startAfter, const char* stopAt, const char* parameter, double optionalValue);
    cParameter* findParameter (const cParameter* startAfter, const char* stopAt, const char* parameter, const cMacAddress& optionalValue);
    cParameter* findParameter (const cParameter* startAfter, const char* stopAt, const char* parameter, const cIpAddress& optionalValue);

#ifdef WITH_UNITTESTS
    static void unitTest ();
#endif

private:
    const char* parseParameters (const char*, bool);
    std::vector<cParameter> list;
    std::vector<bool> used;
    const char* parseError;
    cDefaultParameter defaultParameter;
};



#endif

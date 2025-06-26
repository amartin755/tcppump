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
#include "uuid.hpp"
#include "formatexception.hpp"
#include "bug.hpp"

class cParameter
{
public:
    cParameter ();
    cParameter (const cParameter&);
    virtual ~cParameter();

    virtual uint64_t    asInt64 (uint64_t rangeBegin = 0, uint64_t rangeEnd = static_cast<int64_t>(0xffffffffffffffff)) const;
    virtual uint32_t    asInt32 (uint32_t rangeBegin = 0, uint32_t rangeEnd = 0xffffffff) const;
    virtual uint16_t    asInt16 (uint16_t rangeBegin = 0, uint16_t rangeEnd = 0xffff) const;
    virtual uint8_t     asInt8  (uint8_t  rangeBegin = 0, uint8_t rangeEnd = 0xff) const;
    virtual double      asDouble(double rangeBegin = DBL_MIN, double rangeEnd = DBL_MAX) const;
    virtual cMacAddress asMac   () const;
    virtual const uint8_t* asStream (size_t& len, size_t maxLen = SIZE_MAX);
    virtual const uint8_t* asEmbedded (bool &isEmbedded, size_t& len);
    virtual cIPv4  asIPv4  () const;
    virtual cIPv6  asIPv6  () const;
    virtual cUUID  asUUID  ()
    {
        size_t len;
        const uint8_t* pUuid = asStream (len, cUUID::stringSize());
        std::string uuidAsString ((const char*)pUuid, len);
        try
        {
            return cUUID::fromString(uuidAsString);
        }
        catch(...)
        {
            throw FormatException (exParFormat, value, (int)valLen);
        }
    }

    std::pair<const char*, size_t> name () const
    {
        return std::pair<const char*, size_t> (parameter, parLen);
    }

    void throwValueException (void) const;

private:
    void clear ();
    int isRandom (bool allowRange) const;
    int isRandomInteger (uint64_t& min, uint64_t& max) const;
    const uint8_t* asStream (bool allowEmbPacket, bool &isEmbedded, size_t& len, size_t maxLen = SIZE_MAX);

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
    virtual const uint8_t* asStream   (size_t&, size_t)
    {
        BUG ("no raw access for optional parameters");
        return NULL;
    }
    virtual const uint8_t* asEmbedded (bool&, size_t&)
    {
        BUG ("no raw access for optional parameters");
        return NULL;
    }
    virtual cIPv4  asIPv4  () const {return ip;}
    virtual cUUID  asUUID () {return cUUID::fromZero();}

private:
    uint32_t    int32;
    cMacAddress mac;
    cIPv4  ip;
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
    cParameter* findParameter (const char* parameter, const cIPv4& optionalValue);
    cParameter* findParameter (const cParameter* startAfter, const char* stopAt, const char* parameter, uint32_t optionalValue);
    cParameter* findParameter (const cParameter* startAfter, const char* stopAt, const char* parameter, double optionalValue);
    cParameter* findParameter (const cParameter* startAfter, const char* stopAt, const char* parameter, const cMacAddress& optionalValue);
    cParameter* findParameter (const cParameter* startAfter, const char* stopAt, const char* parameter, const cIPv4& optionalValue);

    typedef std::vector<cParameter>::iterator iterator;
    typedef std::vector<cParameter>::const_iterator const_iterator;
    iterator begin () { return list.begin (); }
    iterator end ()   { return list.end (); }
    const_iterator cbegin () const { return list.cbegin(); }
    const_iterator cend () const   { return list.cend(); }

    void setParameterUsed (const cParameter* s, bool val)
    {
        used.at(s->index) = val;
    }

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

/**
 * NETNAG <https://github.com/amartin755/netnag>
 * Copyright (C) 2012-2016 Andreas Martin (netnag@mailbox.org)
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

#ifndef PARAMETERLIST_HPP_
#define PARAMETERLIST_HPP_

#include <vector>

#include "protocoltypes.hpp"
#include "ipaddress.hpp"
#include "formatexception.hpp"


class cParameter
{
public:
	cParameter ();
	cParameter (const cParameter&);

	virtual uint32_t    asInt32 (uint32_t rangeBegin = 0, uint32_t rangeEnd = 0xffffffff) const;
	virtual uint16_t    asInt16 (uint16_t rangeBegin = 0, uint16_t rangeEnd = 0xffff) const;
	virtual uint8_t     asInt8  (uint8_t  rangeBegin = 0, uint8_t rangeEnd = 0xff) const;
	virtual mac_t       asMac   () const;
	virtual const char* asRaw   (size_t& len) const;
	virtual cIpAddress  asIPv4  () const;

private:
	const char* parameter;
	size_t      parLen;
	const char* value;
	size_t      valLen;
	int         index;

	friend class cParameterList;
};


class cDefaultParameter : public cParameter
{
	friend class cParameterList;

public:
	virtual uint32_t    asInt32 (uint32_t, uint32_t) const {return int32;}
	virtual uint16_t    asInt16 (uint16_t, uint16_t) const {return (uint16_t)int32;}
	virtual uint8_t     asInt8  (uint8_t,  uint8_t) const {return (uint8_t)int32;}
	virtual mac_t       asMac   () const {return mac;}
	virtual const char* asRaw   (size_t&) const
	{
		assert ("no raw access for optional parameters" == 0);
		return NULL;
	}
	virtual cIpAddress  asIPv4  () const {return ip;}

private:
	uint32_t   int32;
	mac_t      mac;
	cIpAddress ip;
};


class cParameterList
{
public:
	cParameterList (const char*);
	bool isValid ();
	const char* getParseError ();
	const cParameter* findParameter (const cParameter* startAfter, const char* stopAt, const char* parameter, bool isOptional = false);
	const cParameter* findParameter (const char* parameter, bool isOptional = false);
	const cParameter* findParameter (const char* parameter, uint32_t optionalValue);
	const cParameter* findParameter (const char* parameter, const mac_t& optionalValue);
	const cParameter* findParameter (const char* parameter, const cIpAddress& optionalValue);
	const cParameter* findParameter (const cParameter* startAfter, const char* stopAt, const char* parameter, uint32_t optionalValue);
	const cParameter* findParameter (const cParameter* startAfter, const char* stopAt, const char* parameter, const mac_t& optionalValue);
	const cParameter* findParameter (const cParameter* startAfter, const char* stopAt, const char* parameter, const cIpAddress& optionalValue);

#ifdef WITH_UNITTESTS
	static void unitTest ();
#endif

private:
	const char* parseParameters (const char*);
	std::vector<cParameter> list;
	const char* parseError;
	cDefaultParameter defaultParameter;
};



#endif

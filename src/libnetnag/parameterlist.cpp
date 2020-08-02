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


#include <cctype>
#include <cstring>
#include <cassert>
#include <cerrno>

#include "parameterlist.hpp"

#include "converter.hpp"



cParameter::cParameter ()
{
	parameter = NULL;
	parLen    = 0;
	value     = NULL;
	valLen    = 0;
	index     = -1;
}


cParameter::cParameter (const cParameter& obj)
{
	parameter = obj.parameter;
	parLen    = obj.parLen;
	value     = obj.value;
	valLen    = obj.valLen;
	index     = obj.index;
}


uint32_t cParameter::asInt32 (uint32_t rangeBegin, uint32_t rangeEnd) const
{
	assert (rangeEnd >= rangeBegin);

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


mac_t cParameter::asMac () const
{
	mac_t mac;

	if (nn::Converter::stringToMac (value, mac, valLen))
	{
		return mac;
	}
	else
	{
		throw FormatException(exParFormat, value);
	}
}


const char* cParameter::asRaw (size_t& len) const
{
	len =  valLen;
	return value;
}


ipv4_t cParameter::asIPv4 () const
{
	ipv4_t ip;
	if (nn::Converter::stringToIpv4 (value, ip, valLen))
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
const cParameter* cParameterList::findParameter (const cParameter* startAfter, const char* stopAt, const char* parameter, bool isOptional)
{
	unsigned n = 0;

	if (startAfter)
		n = startAfter->index >= 0 ? startAfter->index + 1 : list.size();
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

const cParameter* cParameterList::findParameter (const char* parameter, bool isOptional)
{
	return findParameter (nullptr, nullptr, parameter, isOptional);
}


const cParameter* cParameterList::findParameter (const cParameter* startAfter, const char* stopAt, const char* parameter, uint32_t optionalValue)
{
	const cParameter* p = findParameter (startAfter, stopAt, parameter, true);
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


const cParameter* cParameterList::findParameter (const cParameter* startAfter, const char* stopAt, const char* parameter, mac_t& optionalValue)
{
	const cParameter* p = findParameter (startAfter, stopAt, parameter, true);
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


const cParameter* cParameterList::findParameter (const char* parameter, uint32_t optionalValue)
{
	return findParameter (nullptr, nullptr, parameter, optionalValue);
}


const cParameter* cParameterList::findParameter (const char* parameter, mac_t& optionalValue)
{
	return findParameter (nullptr, nullptr, parameter, optionalValue);
}


const char* cParameterList::parseParameters (const char* parameters)
{
	const char* p = parameters;
	cParameter v;
	enum {FIND_PARAMETER, PAR_START, PAR_END, OPERATOR, VAL_START, VAL_END}state;
	const char MAGIC_SYMBOL = '.';

	list.clear ();

	/*
	 * Parsing rules
	 * - parameter name start with MAGIC_SYMBOL (a whitespace before is mandatory)
	 * - parameter name ends with first whitespace or '='
	 * - value starts with first non-whitespace after the '='
	 * - value ends with first whitespace
	 */

	state = FIND_PARAMETER;

	while (*p != '\0')
	{

		switch (state)
		{
		case FIND_PARAMETER: // warten auf symbol
			if (*p == MAGIC_SYMBOL)
			{
				state = PAR_START;
			}
			else if (isspace (*p))
			{
				state = FIND_PARAMETER;
			}
			else
			{
				return p;//FEHLER
			}
			break;
		case PAR_START: // symbol gefunden, nun muss ein Zeichen kommen
			if (isalpha (*p))
			{
				v.parameter = p;
				v.parLen++;
				state = PAR_END;
			}
			else
			{
				return p;//FEHLER
			}
			break;
		case PAR_END: // warten auf das Ende des Parameters
			if (isalnum (*p) || *p == '_') // underscore is allowed in parameter name
			{
				v.parLen++;
			}
			else if (isspace (*p))
			{
				state = OPERATOR;
			}
			else if (*p == '=')
			{
				state = VAL_START;
			}
			else
			{
				return p;//FEHLER
			}
			break;
		case OPERATOR: // warten auf =
			if (isspace (*p))
			{
				state = OPERATOR;
			}
			else if (*p == '=')
			{
				state = VAL_START;
			}
			else
			{
				return p;//FEHLER
			}
			break;
		case VAL_START: // warten auf Wertanfang
			if (isspace (*p))
			{
				state = VAL_START;
			}
			else if (isalnum (*p))
			{
				v.value = p;
				v.valLen++;
				state = VAL_END;
			}
			else
			{
				return p;//FEHLER
			}
			break;
		case VAL_END: // warten auf Wertende
			if (isalnum (*p) || *p == '.' || *p == ':')
			{
				v.valLen++;
			}
			else if (isspace (*p))
			{
				v.index = list.size ();
				list.push_back (v);
				state = FIND_PARAMETER;
				v.parameter = v.value = NULL;
				v.parLen = v.valLen = 0;
			}
			else
			{
				return p;//FEHLER
			}
			break;
		}
		p++;
	}

	if (state == VAL_END)
	{
		v.index = list.size ();
		list.push_back (v);
		state = FIND_PARAMETER;
		v.parameter = v.value = NULL;
		v.parLen = v.valLen = 0;
	}

	return state == FIND_PARAMETER ? NULL : p;
}


#ifdef WITH_UNITTESTS
#include "console.hpp"
void cParameterList::unitTest ()
{
	nn::Console::PrintDebug("-- " __FILE__ " --\n");

	bool catched = false;

	try
	{
		cParameterList obj ("     .first=100 .second = 200 .third   =300");
		assert (obj.isValid ());
		assert (obj.findParameter("first")->asInt32()  == (uint32_t)100);
		assert (obj.findParameter("second")->asInt32() == (uint32_t)200);
		assert (obj.findParameter("third")->asInt32()  == (uint32_t)300);
	}
	catch (FormatException& e)
	{
		assert (0);
	}

	{
		cParameterList obj (".first=100 .second = 200 .third   =300");
		assert (obj.isValid ());
		try
		{
			catched = false;
			assert (obj.findParameter("first")->asInt32()  == (uint32_t)100);
			assert (obj.findParameter("second")->asInt32() == (uint32_t)200);
			assert (obj.findParameter("third")->asInt32()  == (uint32_t)300);
		}
		catch (FormatException& e)
		{
			assert (0);
		}

		try
		{
			catched = false;
			obj.findParameter("third")->asMac();
		}
		catch (FormatException& e)
		{
			catched = true;
			assert (e.what () == exParFormat);
			assert (e.value ());
		}
		assert (catched);

		try
		{
			catched = false;
			obj.findParameter("third")->asIPv4();
		}
		catch (FormatException& e)
		{
			catched = true;
			assert (e.what () == exParFormat);
			assert (e.value ());
		}
		assert (catched);

		try
		{
			catched = false;
			obj.findParameter("f");
		}
		catch (FormatException& e)
		{
			catched = true;
			assert (e.what () == exParUnknown);
			assert (e.value ());
			assert (!strcmp ("f", e.value ()));
		}
		assert (catched);

		try
		{
			catched = false;
			obj.findParameter("firstfirst");
		}
		catch (FormatException& e)
		{
			catched = true;
			assert (e.what () == exParUnknown);
			assert (e.value ());
			assert (!strcmp ("firstfirst", e.value ()));
		}
		assert (catched);

		try
		{
			assert (obj.findParameter("first")->asInt32(100, 100)  == (uint32_t)100);
		}
		catch (FormatException& e)
		{
			assert (0);
		}

		try
		{
			catched = false;
			obj.findParameter("first")->asInt32(101, 102);
		}
		catch (FormatException& e)
		{
			catched = true;
			assert (e.what () == exParRange);
			assert (e.value ());
		}
		assert (catched);

	}

	try
	{
		cParameterList obj (".first=100 .firstsecond = 200 .third   =300");
		assert (obj.findParameter("first")->asInt32()  == (uint32_t)100);
		assert (obj.findParameter("firstsecond")->asInt32()  == (uint32_t)200);
		assert (obj.isValid ());
	}
	catch (FormatException& e)
	{
		assert (0);
	}

	{
		cParameterList obj (".first=100.firstsecond = 200 .third   =300");
		assert (!obj.isValid ());
	}
	{
		cParameterList obj ("d.first=100.second =");
		assert (!obj.isValid ());
	}
	{
		cParameterList obj ("d.first=100");
		assert (!obj.isValid ());
	}
	{
		cParameterList obj (".=123");
		assert (!obj.isValid ());
	}
	try
	{
		cParameterList obj ("  .first=123");
		assert (obj.isValid ());
		assert (obj.findParameter("first")->asInt32()  == (uint32_t)123);
	}
	catch (FormatException& e)
	{
		assert (0);
	}
	{
		cParameterList obj ("$.dkfjsdf=sd.djhdslk..0=0sd sdlfkjf");
		assert (!obj.isValid ());
	}
	{
		cParameterList obj (".long=100 .ipv4 = 1.2.3.4 .mac =12:34:56:78:9A:BC .payload=012345");
		try
		{
			assert (obj.isValid ());
			assert (obj.findParameter("long")->asInt32()  == (uint32_t)100);
			assert (obj.findParameter("ipv4")->asIPv4()  == (ipv4_t)0x04030201);
			mac_t mac2 = {0x12,0x34,0x56,0x78,0x9a,0xbc};
			mac_t mac = obj.findParameter("mac")->asMac();
			assert (!memcmp (&mac, &mac2, sizeof (mac2)));
			size_t len = 0;
			assert (!strcmp (obj.findParameter("payload")->asRaw(len), "012345"));
			assert (len == 6);
		}
		catch (FormatException& e)
		{
			assert (0);
		}
	}
	{
		cParameterList obj (".first=0xFFFF .second=0x10000 .toolong=0x100000000");
		assert (obj.isValid ());
		try
		{
			assert (obj.findParameter("first")->asInt32()  == (uint32_t)0x0000ffff);
			assert (obj.findParameter("first")->asInt32(200, 0xffff)  == (uint32_t)0x0000ffff);
			assert (obj.findParameter("first")->asInt32(0xffff, 0xffff)  == (uint32_t)0x0000ffff);
			assert (obj.findParameter("first")->asInt16()  == (uint16_t)0x0000ffff);
			assert (obj.findParameter("second")->asInt32()  == (uint32_t)0x00010000);
			assert (obj.findParameter("second")->asInt32(0x00010000)  == (uint32_t)0x00010000);
		}
		catch (FormatException& e)
		{
			assert (0);
		}
		try
		{
			catched = false;
			obj.findParameter("first")->asInt8();
		}
		catch (FormatException& e)
		{
			catched = true;
			assert (e.what () == exParRange);
			assert (e.value ());
		}
		assert (catched);
		try
		{
			catched = false;
			obj.findParameter("second")->asInt16();
		}
		catch (FormatException& e)
		{
			catched = true;
			assert (e.what () == exParRange);
			assert (e.value ());
		}
		assert (catched);
		try
		{
			catched = false;
			obj.findParameter("toolong")->asInt32();
		}
		catch (FormatException& e)
		{
			catched = true;
			assert (e.what () == exParRange);
			assert (e.value ());
		}
		assert (catched);
		{
			cParameterList obj (".dk.fjsdf=12");
			assert (!obj.isValid ());
		}
	}
	{
		cParameterList obj (".first=0x1 .first=0x2 .first=0x3 ");
		assert (obj.isValid ());
		try
		{
			const cParameter* par = nullptr;
			assert (obj.findParameter("first")->asInt32()  == (uint32_t)1);

			par = obj.findParameter("first");
			assert (par->asInt32()  == (uint32_t)1);
			par = obj.findParameter(par, nullptr, "first");
			assert (par->asInt32()  == (uint32_t)2);
			par = obj.findParameter(par, nullptr, "first");
			assert (par->asInt32()  == (uint32_t)3);
			par = obj.findParameter(par, nullptr, "first", true);
			assert (!par);
		}
		catch (FormatException& e)
		{
			assert (0);
		}
	}
	{
		cParameterList obj (".second=0x10 .first=0x1 .second=0x10 .first=0x2 .second=0x20 .first=0x3 .second=0x30");
		assert (obj.isValid ());
		try
		{
			const cParameter* par = nullptr;
			assert (obj.findParameter("first")->asInt32()  == (uint32_t)1);

			par = obj.findParameter("first");
			assert (par->asInt32()  == (uint32_t)1);
			assert (obj.findParameter(par, nullptr, "second")->asInt32()  == (uint32_t)0x10);

			par = obj.findParameter(par, nullptr, "first");
			assert (par->asInt32()  == (uint32_t)2);
			assert (obj.findParameter(par, nullptr, "second")->asInt32()  == (uint32_t)0x20);

			par = obj.findParameter(par, nullptr, "first");
			assert (par->asInt32()  == (uint32_t)3);
			assert (obj.findParameter(par, nullptr, "second")->asInt32()  == (uint32_t)0x30);

			par = obj.findParameter(par, nullptr, "first", true);
			assert (!par);
		}
		catch (FormatException& e)
		{
			assert (0);
		}
	}
	{
		cParameterList obj (".second=0x10 .first=0x1 .first=0x2 .second=0x20 .first=0x3 .second=0x30");
		assert (obj.isValid ());
		try
		{
			const cParameter* par = nullptr;

			par = obj.findParameter("first");
			assert (par->asInt32()  == (uint32_t)1);
			assert (!obj.findParameter(par, "first", "second", true));

			par = obj.findParameter(par, nullptr, "first");
			assert (par->asInt32()  == (uint32_t)2);
			assert (obj.findParameter(par, "first", "second")->asInt32()  == (uint32_t)0x20);

			par = obj.findParameter(par, nullptr, "first");
			assert (par->asInt32()  == (uint32_t)3);
			assert (obj.findParameter(par, nullptr, "second")->asInt32()  == (uint32_t)0x30);

			par = obj.findParameter(par, nullptr, "first", true);
			assert (!par);
		}
		catch (FormatException& e)
		{
			assert (0);
		}
	}
}
#endif

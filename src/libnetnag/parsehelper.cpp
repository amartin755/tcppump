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
#include <cassert>

#include "parsehelper.hpp"


const char* cParseHelper::skipWhitespaces (const char* p)
{
    while (isspace (*p))
        p++;

    return p;
}

const char* cParseHelper::nextTokenStart (const char* p, bool alpha, bool num, const char* accept)
{
    // ignore whitespaces
    p = skipWhitespaces(p);

    if ((alpha && isalpha (*p)) || (num && isdigit (*p)) || (accept && isOneOf (*p, accept)))
    	return p;

    return nullptr;
}

const char* cParseHelper::nextTokenEnd (const char* p, bool alpha, bool num, const char* accept)
{
	while (*p != '\0')
	{
	    if ((alpha && isalpha (*p)) || (num && isdigit (*p)) || (accept && isOneOf (*p, accept)))
	    	p++;
	    else
	    	return p;
	}
	return p;
}

const char* cParseHelper::nextCharIgnoreWhitspaces (const char* p, char c)
{
	p = skipWhitespaces(p);
	return *p == c ? p : nullptr;
}

int cParseHelper::isOneOf (char c, const char* accept)
{
	assert (accept);

	while (*accept != '\0')
		if (*accept++ == c)
			return 1;

	return 0;
}


#ifdef WITH_UNITTESTS

#include "console.hpp"

void cParseHelper::unitTest ()
{
	nn::Console::PrintDebug("-- " __FILE__ " --\n");

	assert (isOneOf ('a', "abcdef"));
	assert (isOneOf ('c', "abcdef"));
	assert (isOneOf ('f', "abcdef"));
	assert (!isOneOf ('r', "abcdef"));
	assert (!isOneOf ('r', ""));

	assert (*skipWhitespaces ("abc") == 'a');
	assert (*skipWhitespaces (" abc") == 'a');
	assert (*skipWhitespaces (" \t\nabc") == 'a');
	assert (*skipWhitespaces ("") == '\0');

	assert (!nextCharIgnoreWhitspaces (" abcd\tef\ng", 'f'));
	assert (!nextCharIgnoreWhitspaces (" abcd\tef\ng", 'b'));
	assert (*nextCharIgnoreWhitspaces (" abcd\tef\ng", 'a') == 'a');

	{
		const char s[] = "abc de-0f  ghi";
		const char* p = nullptr;
		p = nextKeyStart (s);
		assert (p == &s[0]);
		p = nextKeyEnd (p);
		assert (p == &s[3]);
		p = nextKeyStart (p);
		assert (p == &s[4]);
		p = nextKeyEnd (p);
		assert (p == &s[9]);
		p = nextKeyStart (p);
		assert (p == &s[11]);
		p = nextKeyEnd (p);
		assert (p == &s[14]);
	}
	{
		const char s[] = "  abc \tde-0f  ghi";
		const char* p = nullptr;
		p = nextKeyStart (s);
		assert (p == &s[2]);
		p = nextKeyEnd (p);
		assert (p == &s[5]);
		p = nextKeyStart (p);
		assert (p == &s[7]);
		p = nextKeyEnd (p);
		assert (p == &s[12]);
		p = nextKeyStart (p);
		assert (p == &s[14]);
		p = nextKeyEnd (p);
		assert (p == &s[17]);
	}
	{
		const char s[] = "a b c ";
		const char* p = nullptr;
		p = nextKeyStart (s);
		assert (p == &s[0]);
		p = nextKeyEnd (p);
		assert (p == &s[1]);
		p = nextKeyStart (p);
		assert (p == &s[2]);
		p = nextKeyEnd (p);
		assert (p == &s[3]);
		p = nextKeyStart (p);
		assert (p == &s[4]);
		p = nextKeyEnd (p);
		assert (p == &s[5]);
	}
	{
		const char s[] = "-a";
		const char* p = nullptr;
		p = nextKeyStart (s);
		assert (!p);
	}
	{
		const char s[] = "5a";
		const char* p = nullptr;
		p = nextKeyStart (s);
		assert (!p);
	}

	// TODO much more detailed tests
}
#endif

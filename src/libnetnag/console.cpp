/**
 * NETNAG <https://github.com/amartin755/netnag>
 * Copyright (C) 2012-2016 Andreas Martin (netnag@mailbox.org)
 *
 * shellout.cpp
 *
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


#include "../libnetnag/console.hpp"

#include <cassert>
#include <cstdarg>
#include <cstdio>


namespace nn
{
	Console::out_level Console::level = Normal;

	void Console::SetPrintLevel (out_level lvl)
	{
		assert ((lvl >= Silent) && (lvl <= Debug));
		level = lvl;
	}

	bool Console::PrintError (const char* format, ...)
	{
		bool ret;
		va_list args;
		va_start (args, format);

		ret = Console::print (Console::Error, format, args);

		va_end (args);
		return ret;
	}
	bool Console::Print (const char* format, ...)
	{
		bool ret;
		va_list args;
		va_start (args, format);

		ret = Console::print (Console::Normal, format, args);

		va_end (args);
		return ret;
	}
	bool Console::PrintVerbose (const char* format, ...)
	{
		bool ret;
		va_list args;
		va_start (args, format);

		ret = Console::print (Console::Verbose, format, args);

		va_end (args);
		return ret;
	}
	bool Console::PrintDebug (const char* format, ...)
	{
		bool ret;
		va_list args;
		va_start (args, format);

		ret = Console::print (Console::Debug, format, args);

		va_end (args);
		return ret;
	}

	bool Console::print (out_level lvl, const char* format, va_list ap)
	{
		if (lvl > level)
			return false;

		 // we always print to stderr to be able to separate piped in/output from console prints
		 return vfprintf (stderr, format, ap) >= 0;
	}

}

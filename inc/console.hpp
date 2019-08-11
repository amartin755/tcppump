/**
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2016 Andreas Martin (netnag@mailbox.org)
 *
 * shellout.hpp
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


#ifndef CONSOLE_HPP_
#define CONSOLE_HPP_

#include <cstdarg>

namespace nn
{



	class Console
	{
	public:
		static bool PrintError (const char* format, ...);
		static bool Print (const char* format, ...);
		static bool PrintVerbose (const char* format, ...);
		static bool PrintDebug (const char* format, ...);

		enum out_level {Silent = 1, Error = 2, Normal = 3, Verbose = 4, Debug = 5};
		static void SetPrintLevel (out_level lvl);

	private:
		static bool print (out_level lvl, const char* format, va_list ap);

	private:
		static out_level level;
	};
}
#endif /* CONSOLE_HPP_ */

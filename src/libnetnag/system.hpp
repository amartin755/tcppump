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


#ifndef SYSTEM_HPP_
#define SYSTEM_HPP_

#include <cstdio> // FILE

#include "protocoltypes.hpp"

namespace nn
{
	class System
	{
	public:
		static int ExecuteProcess (const char* cmdline);
		static const char* GetTmpFilePath ();
		static FILE* OpenTmpFile (const char* mode = "rb", const char** path = NULL);
		static bool Diff (const char* file1, const char* file2);
		static bool ResolveMacForIpAddress (ipv4_t ip, mac_t &mac);
		static void Sleep (unsigned seconds);

		static void unitTest ();
	};
}
#endif /* SYSTEM_HPP_ */

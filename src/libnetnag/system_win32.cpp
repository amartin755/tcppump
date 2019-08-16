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

#include <cassert>
#include <cstdlib>
#include <string>
#include <windows.h>
#include <winsock2.h>
#include <iphlpapi.h>

#include "system.hpp"
#include "console.hpp"
#include "converter.hpp"

namespace nn
{
#if 0
int System::ExecuteProcess (const char* cmdline)
{
    DWORD exitCode = -1;
	char execPath[MAX_PATH + 1] = {0};

	if (GetModuleFileName (NULL, execPath, MAX_PATH))
	{
		char* p = strrchr (execPath, '\\');
		if (p++)
			*p = '\0';

		strncat (execPath, cmdline, sizeof (execPath) - strlen (execPath) - 1);

		STARTUPINFO si = {0};
		PROCESS_INFORMATION pi = {0};

		si.cb = sizeof(si);

		// Start the child process.
		if (CreateProcess (NULL,   // No module name (use command line)
				execPath,        // Command line
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			FALSE,          // Set handle inheritance to FALSE
			0,              // No creation flags
			NULL,           // Use parent's environment block
			NULL,           // Use parent's starting directory
			&si,            // Pointer to STARTUPINFO structure
			&pi ))           // Pointer to PROCESS_INFORMATION structure
		{
			// Wait until child process exits.
			WaitForSingleObject (pi.hProcess, INFINITE);

			// Get the exit code.
			GetExitCodeProcess (pi.hProcess, &exitCode);

			// Close process and thread handles.
			CloseHandle (pi.hProcess);
			CloseHandle (pi.hThread);
		}
		else
		{
			Console::PrintError ("CreateProcess failed (%d)\n", (int)GetLastError() );
		}
    }
    else
    {
    	Console::PrintError ("GetModuleFileName failed (%d)\n", (int)GetLastError() );
    }

    return (int)exitCode;
}

const char* System::GetTmpFilePath ()
{
	// note, _tempnam allocs a buffer on the heap. The caller has to free this buffer!
	const char* tmpPath = _tempnam (NULL, "NETNAG");
	if (!tmpPath)
	{
		Console::PrintError ("could not determine temp file name\n");
	}
	return tmpPath;
}

FILE* System::OpenTmpFile (const char* mode, const char** path)
{
	FILE* fp = NULL;
	const char* tmpPath = GetTmpFilePath ();

	if (tmpPath)
	{
		fp = fopen (tmpPath, mode);
		if (!fp)
		{
			Console::PrintError ("could not create temp file %s\n", tmpPath);
			free ((void*)tmpPath);
			return NULL;
		}

		if (path)
			*path = tmpPath;
	}
	return fp;
}

bool System::Diff (const char* file1, const char* file2)
{
	std::string cmdline ("fc /b ");

	cmdline += file1;
	cmdline += " ";
	cmdline += file2;
	return !system (cmdline.c_str());
}

bool System::ResolveMacForIpAddress (ipv4_t ip, mac_t &mac)
{
	ULONG macLen = sizeof (mac);

	DWORD ret = SendARP(ip, INADDR_ANY, (PULONG)&mac, (PULONG)&macLen);

	ip = ntohl (ip);
	if (ret == NO_ERROR && macLen == sizeof (mac))
	{
		return true;
	}
	else
	{
		Console::PrintError ("Could not resolve MAC for IPv4 address %d.%d.%d.%d\n",
				(ip >> 24) & 0xff, (ip >> 16) & 0xff, (ip >> 8) & 0xff, ip & 0xff);
		Console::PrintDebug (" errorcode = %u\n", (unsigned)ret);
	}

	return false;
}
#endif
void System::Sleep (unsigned seconds)
{
	::Sleep (seconds * 1000);
}

void System::unitTest ()
{
	// TODO

}

}

///////////////////////////////////////////////////////////////////////////////
// 
// Author: Wong Yiu Kwan, Law Chun Kit
// 
// Adapted From "DebugEvents.cpp" by Oleg Starodumov (www.debuginfo.com)
// Modified to use Windbg API
// 
///////////////////////////////////////////////////////////////////////////////
// Original header
///////////////////////////////////////////////////////////////////////////////
//	//
//	// DebugEvents.cpp
//	// 
//	// Author: Oleg Starodumov (www.debuginfo.com)
//	//
//	//
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <fileapi.h>

#include <stdio.h>
#include <malloc.h>

#include <tchar.h>
#include <string>

#include "BaseConfig.h"
#include "ChromeDBGCallback.h"


// Change to set dynamicly
// Offset to OnExtensionSystemReady() method in Loaded DLL 
// Calculation:  [fileOffset] - [SizeOfHeaders] + [.text Section Address]
//#define CHROME_ExSysRdy_OFFSET 0x144DDAE


///////////////////////////////////////////////////////////////////////////////
// Type definitions
//

typedef std::basic_string<TCHAR>  TString;


///////////////////////////////////////////////////////////////////////////////
// Function declarations
//

	// Start debugging
bool StartProcess(const TString& FileName, const TString& CmdLine, IDebugClient* g_client);

// Get debuggee command line
bool GetDebuggeeCommandLine(int argc, TCHAR* argv[], int StartIndex, TString& CmdLine);

// Enable debug privilege
bool EnableDebugPrivilege(bool Enable);

// Help and logo printers 
void PrintHelp();
void PrintLogo();


///////////////////////////////////////////////////////////////////////////////
// main() function 
//

int _tmain(int argc, TCHAR* argv[])
{
	// Command line parameter constants 

	const TCHAR* ccAttach = _T("-p");
	const TCHAR* ccHelp = _T("-?");


	IDebugClient* g_client = nullptr;

	IDebugControl* g_control = nullptr;
	HRESULT g_hresult;
	g_hresult = DebugCreate(__uuidof(IDebugClient), (void**)&g_client);
	g_hresult = g_client->QueryInterface(__uuidof(IDebugControl3), (void**)&g_control);


	IDebugClient* g_client5 = nullptr;
	g_hresult = g_client->QueryInterface(__uuidof(IDebugClient5), (void**)&g_client5);


	// Print logo 

	PrintLogo();


	// Enable debug privilege

	EnableDebugPrivilege(true);


	// Obtain and check command line parameters 

	if (argc < 2)
	{
		_tprintf(_T("argc:  %d\n argv[0] %s\n"), argc, argv[0]);
		PrintHelp();
		return 0;
	}

	if (_tcscmp(argv[1], ccHelp) == 0)
	{
		PrintHelp();
		return 0;
	}
	else
	{
		// Launch requested

		TString CmdLine;

		if (!GetDebuggeeCommandLine(argc, argv, 1, CmdLine) || CmdLine.empty())
		{
			// Something wrong with the command line...
			PrintHelp();
			return 0;
		}

		_tprintf(_T("Mode:     Launch\n"));
		_tprintf(_T("Command:  %s\n\n"), CmdLine.c_str());

		if (!StartProcess(_T(""), CmdLine, g_client))
		{
			_tprintf(_T("StartProcess() failed.\n"));
			return 0;
		}

	}

	ChromeDBGCallback myCallback(g_client);

	g_client5->SetEventCallbacks(&myCallback);

	while (myCallback.listening) {
		g_control->WaitForEvent(0, INFINITE);
	}


	return 0;
}



bool StartProcess(const TString& FileName, const TString& CmdLine, IDebugClient* g_client)
{
	PSTR cCmdLine = nullptr;
	if (CmdLine.length() <= 0) {
		_tprintf(_T("Too few Argument\n"));
		PrintHelp();
		exit(1);
	}

	cCmdLine = (PSTR)_alloca((CmdLine.length() * 2 + 1) * sizeof(PCHAR));
	WideCharToMultiByte(CP_ACP, 0, CmdLine.c_str(), CmdLine.length(), cCmdLine, CmdLine.length() * 2, NULL, NULL);

	// Create the process 

	STARTUPINFO si = { sizeof(si) };

	PROCESS_INFORMATION pi = { NULL, NULL, 0, 0 };

	HRESULT hRes = g_client->CreateProcessW(0, cCmdLine, CREATE_NEW_CONSOLE | DEBUG_ONLY_THIS_PROCESS);
	if (hRes != S_OK)
	{
		_tprintf(_T("CreateProcess() failed. Error: %u \n"), GetLastError());
		_ASSERTE(!_T("CreateProcess() failed."));
		return false;
	}

	return true;

}


bool GetDebuggeeCommandLine(int argc, TCHAR* argv[], int StartIndex, TString& CmdLine)
{
	// Cleanup the [out] parameter

	CmdLine = _T("");


	// Check parameters 

	_ASSERTE(argc > 0);
	_ASSERTE(argv != 0);
	_ASSERTE(StartIndex >= 0);
	_ASSERTE(StartIndex < argc);

	if ((argc <= 0) || (argv == 0) || (StartIndex < 0) || (StartIndex >= argc))
	{
		_ASSERTE(_T("Invalid parameter."));
		return false;
	}


	// Concatenate the parameters to the destination string 

	while (StartIndex < argc)
	{
		bool HasSpace = (_tcschr(argv[StartIndex], _T(' ')) != NULL);

		if (HasSpace)
			CmdLine += _T("\"");

		CmdLine += argv[StartIndex];

		if (HasSpace)
			CmdLine += _T("\"");

		if (StartIndex < argc)
			CmdLine += _T(" ");

		StartIndex++;
	}


	// Complete 

	return true;

}


///////////////////////////////////////////////////////////////////////////////
// EnableDebugPrivilege function 
// 
// This function enables or disables debug privilege
//

bool EnableDebugPrivilege(bool Enable)
{
	bool Success = false;

	HANDLE hToken = NULL;

	DWORD ec = 0;

	do
	{
		// Open the process' token

		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
		{
			ec = GetLastError();
			_ASSERTE(!_T("OpenProcessToken() failed."));
			break;
		}


		// Lookup the privilege value 

		TOKEN_PRIVILEGES tp;

		tp.PrivilegeCount = 1;

		if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid))
		{
			ec = GetLastError();
			_ASSERTE(!_T("LookupPrivilegeValue() failed."));
			break;
		}


		// Enable/disable the privilege

		tp.Privileges[0].Attributes = Enable ? SE_PRIVILEGE_ENABLED : 0;

		if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL))
		{
			ec = GetLastError();
			_ASSERTE(!_T("AdjustPrivilegeValue() failed."));
			break;
		}


		// Success 

		Success = true;

	} while (0);


	// Cleanup

	if (hToken != NULL)
	{
		if (!CloseHandle(hToken))
		{
			ec = GetLastError();
			_ASSERTE(!_T("CloseHandle() failed."));
		}
	}


	// Complete 

	return Success;

}


///////////////////////////////////////////////////////////////////////////////
// Help and logo printers 
//

void PrintHelp()
{
	_tprintf(_T("Usage:\n"));
	_tprintf(_T("  DebugEvents <CmdLine>  launch the process\n"));
	_tprintf(_T("\n"));
}

void PrintLogo()
{
	_tprintf(_T("Credit:\n\n"));
	_tprintf(_T("Adapted from:\n\n"));
	_tprintf(_T("	DebugEvents sample application\n"));
	_tprintf(_T("	Author: Oleg Starodumov (www.debuginfo.com)\n\n"));
}

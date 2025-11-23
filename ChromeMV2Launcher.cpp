///////////////////////////////////////////////////////////////////////////////
// 
// Author: Wong Yiu Kwan, Law Chun Kit
// 
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>

#include <stdio.h>
#include <malloc.h>

#include <tchar.h>
#include <string>

#include "ChromeMV2Launcher.h"
#include "ChromeDBGCallback.h"
#include "ChromeDLLScanner.h"

constexpr const char* HELP_SWITCH = "-?";
constexpr const char* HELP_USAGE = "Usage: ChromeMV2Launcher <path to chrome.dll> <path to chrome.exe> [chrome.exe launch args]\n";

int main(int argc, char* argv[])
{
	if (argc >= 2 && strncmp(argv[1], HELP_SWITCH, sizeof(HELP_SWITCH)) == 0)
	{
		printf(HELP_USAGE);
		return 0;
	}

	// parse args
	if (argc < 3)
	{
		printf("Too few arguments\n");
		printf(HELP_USAGE);
		return 0;
	}


	UINT64 breakpointOffset = scanTgtBreakPointOffset(argv[1]);

	int dbgArgc = argc - 1;
	char** dbgArgv = argv + 1;

	IDebugClient* g_client = nullptr;
	IDebugControl* g_control = nullptr;

	// init debug clients and control object
	HRESULT g_hresult;
	g_hresult = DebugCreate(__uuidof(IDebugClient), (void**)&g_client);
	g_hresult = g_client->QueryInterface(__uuidof(IDebugControl3), (void**)&g_control);

	IDebugClient* g_client5 = nullptr;
	g_hresult = g_client->QueryInterface(__uuidof(IDebugClient5), (void**)&g_client5);



	
	
	std::string CmdLine;
	if (!GetDebuggeeCommandLine(dbgArgc, dbgArgv, 1, CmdLine) || CmdLine.empty())
	{
		// invalid command line
		printf(HELP_USAGE);
		return 0;
	}

	// launch Debugee
	printf("Command line:  %s\n\n", CmdLine.c_str());
	if (!StartDebugeeProcess(CmdLine, g_client))
	{
		printf("StartDebugeeProcess() failed.\n");
		return 0;
	}
	

	ChromeDBGCallback myCallback(g_client, breakpointOffset);

	g_client5->SetEventCallbacks(&myCallback);

	while (myCallback.listening) {
		g_control->WaitForEvent(0, INFINITE);
	}

	return 0;
}



bool StartDebugeeProcess(std::string CmdLine, IDebugClient* g_client)
{
	char* cCmdLine = nullptr;
	if (CmdLine.length() <= 0) {
		printf("Too few Argument\n");
		exit(1);
	}

	cCmdLine = (char*)_malloca(CmdLine.length() * sizeof(char)+2);
	if (cCmdLine == nullptr) {
		printf("Failed to allocate CmdLine buffer");
		exit(1);
	}

	strncpy_s(cCmdLine, CmdLine.length()+1, CmdLine.c_str(), CmdLine.length());

	// Create the process 
	HRESULT hRes = g_client->CreateProcess(0, cCmdLine, CREATE_NEW_CONSOLE | DEBUG_ONLY_THIS_PROCESS);
	_freea(cCmdLine);

	if (hRes != S_OK)
	{
		printf("CreateProcess() failed. Error: 0x%x \n", GetLastError());
		return false;
	}
	return true;
}

// Adapted From "DebugEvents.cpp" by Oleg Starodumov(www.debuginfo.com)
// Build Command Line for debugee. Modified to use ASCII strings.
bool GetDebuggeeCommandLine(int argc, char* argv[], int StartIndex, std::string& CmdLine)
{
	// Cleanup the [out] parameter
	CmdLine = "";

	// Check parameters 
	_ASSERTE(argc > 0);
	_ASSERTE(argv != 0);
	_ASSERTE(StartIndex >= 0);
	_ASSERTE(StartIndex < argc);

	if ((argc <= 0) || (argv == 0) || (StartIndex < 0) || (StartIndex >= argc))
	{
		return false;
	}

	// Concatenate the parameters to the destination string 

	while (StartIndex < argc)
	{
		bool HasSpace = (strrchr(argv[StartIndex], ' ') != NULL);

		if (HasSpace)
			CmdLine += "\"";

		CmdLine += argv[StartIndex];

		if (HasSpace)
			CmdLine += "\"";

		if (StartIndex < argc)
			CmdLine += " ";

		StartIndex++;
	}

	// Complete 

	return true;

}

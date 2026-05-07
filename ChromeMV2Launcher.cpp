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
constexpr const char* DRYRUN_SWITCH = "--dryrun";
constexpr const char* HEADED_SWITCH = "--headed";

constexpr const char* HELP_USAGE = "\
Usage: ChromeMV2Launcher [--dryrun | --headed] <path to chrome.dll> <path to chrome.exe> [chrome.exe launch args]\n\
-?: display this help menu\n\
--dryrun: Scan chrome.dll without launching Chrome\n\
--headed: Run with console window logging debug info\
";


int main(int argc, char* argv[])
{
	if (argc >= 2 && strncmp(argv[1], HELP_SWITCH, sizeof(HELP_SWITCH)) == 0)
	{
		printf(HELP_USAGE);
		return 0;
	}

	boolean dryRun = false;
	boolean headless = true;
	int argsOffset = 1;
	if (strncmp(argv[1], "-", 1) == 0)
	{
		if (strncmp(argv[1], DRYRUN_SWITCH, sizeof(DRYRUN_SWITCH)) == 0)
		{
			dryRun = true;
			argsOffset++;
		}
		else if (strncmp(argv[1], HEADED_SWITCH, sizeof(HEADED_SWITCH)) == 0)
		{
			headless = false;
			argsOffset++;
		}
		else {
			fprintf(stderr, "Unknown option: %s\n", argv[1]);
			fprintf(stderr, HELP_USAGE);
			return 1;
		}
	}

	// parse args
	if (argc < 3 + dryRun ? 1 : 0)
	{
		fprintf(stderr, "Too few arguments\n");
		fprintf(stderr, HELP_USAGE);
		return 1;
	}


	UINT64 breakpointOffset = scanTgtBreakPointOffset(argv[argsOffset]);

	int dbgArgc = argc - argsOffset;
	char** dbgArgv = argv + argsOffset;


	
	
	std::string CmdLine;
	if (!GetDebuggeeCommandLine(dbgArgc, dbgArgv, 1, CmdLine) || CmdLine.empty())
	{
		// invalid command line
		fprintf(stderr, HELP_USAGE);
		return 1;
	}

	if (dryRun) {
		printf(CmdLine.c_str());
		return 0;
	}

	IDebugClient* g_client = nullptr;
	IDebugControl* g_control = nullptr;

	// init debug clients and control object
	HRESULT g_hresult;
	g_hresult = DebugCreate(__uuidof(IDebugClient), (void**)&g_client);
	g_hresult = g_client->QueryInterface(__uuidof(IDebugControl3), (void**)&g_control);

	IDebugClient* g_client5 = nullptr;
	g_hresult = g_client->QueryInterface(__uuidof(IDebugClient5), (void**)&g_client5);

	// launch Debugee
	printf("Command line:  %s\n\n", CmdLine.c_str());
	if (!StartDebugeeProcess(CmdLine, g_client))
	{
		fprintf(stderr, "StartDebugeeProcess() failed.\n");
		return 0;
	}
	

	ChromeDBGCallback myCallback(g_client, breakpointOffset);

	g_client5->SetEventCallbacks(&myCallback);
	
	if (headless) {
		::SetForegroundWindow(::GetConsoleWindow());
		::ShowWindow(::GetForegroundWindow(), SW_HIDE);
	}
	
	
	while (myCallback.listening) {
		g_control->WaitForEvent(0, INFINITE);
	}

	return 0;
}



bool StartDebugeeProcess(std::string CmdLine, IDebugClient* g_client)
{
	char* cCmdLine = nullptr;
	if (CmdLine.length() <= 0) {
		fprintf(stderr, "Too few Argument\n");
		exit(1);
	}

	cCmdLine = (char*)_malloca(CmdLine.length() * sizeof(char)+2);
	if (cCmdLine == nullptr) {
		fprintf(stderr, "Failed to allocate CmdLine buffer");
		exit(1);
	}

	strncpy_s(cCmdLine, CmdLine.length()+1, CmdLine.c_str(), CmdLine.length());

	// Create the process 
	HRESULT hRes = g_client->CreateProcess(0, cCmdLine, CREATE_NEW_CONSOLE | DEBUG_ONLY_THIS_PROCESS);
	_freea(cCmdLine);

	if (hRes != S_OK)
	{
		fprintf(stderr, "CreateProcess() failed. Error: 0x%x \n", GetLastError());
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

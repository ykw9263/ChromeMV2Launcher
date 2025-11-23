#pragma once
#include <string>
#include "BaseConfig.h"

// Launch debuggee
bool StartDebugeeProcess(std::string CmdLine, IDebugClient* g_client);

// Adapted From "DebugEvents.cpp" by Oleg Starodumov(www.debuginfo.com)
// Build Command Line for debugee. Modified to use ASCII strings.
bool GetDebuggeeCommandLine(int argc, char* argv[], int StartIndex, std::string& CmdLine);



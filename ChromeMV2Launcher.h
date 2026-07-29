#pragma once
#include <string>
#include "BaseConfig.h"

// Launch debuggee
bool StartDebugeeProcess(std::string CmdLine, IDebugClient* g_client);

// Adapted From "DebugEvents.cpp" by Oleg Starodumov(www.debuginfo.com)
// Build Command Line for debugee. Modified to use ASCII strings.
bool GetDebuggeeCommandLine(int argc, char* argv[], int StartIndex, std::string& CmdLine);

bool StrVGetDebuggeeCommandLine(std::vector<std::string> args, int StartIndex, std::string& CmdLine);


// TODO: move this somewhere else
// chrome.dll normally lies within 2 layer of folders
#define SEARCH_DEPTH 2
std::optional<std::string> search_file(const std::filesystem::path& folder_path, const std::string& target_filename);
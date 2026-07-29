///////////////////////////////////////////////////////////////////////////////
// 
// Author: Wong Yiu Kwan, Law Chun Kit
// 
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <filesystem>

#include <stdio.h>
#include <malloc.h>

#include <tchar.h>
#include <string>


#include "ChromeMV2Launcher.h"
#include "ChromeDBGCallback.h"
#include "ChromeDLLScanner.h"

#include <argparse/argparse.hpp>


int main(int argc, char* argv[])
{
	argparse::ArgumentParser program("ChromeMv2Launcher");
	program.add_argument("--dryrun").help("Scan chrome.dll for inject point without launching Chrome").flag();
	program.add_argument("-v", "--verbose").help("Run with console window logging debug info").flag();

	program.add_argument("--dll").help("path to chrome.dll");
	program.add_argument("--exe").help("path to chrome.exe");

	program.add_argument("installation").help("Path to Chrome installation").required();
	program.add_argument("--args").help("Arguments to be passed to Chrome").remaining().default_value(std::vector<std::string>{});
	
	try {
		program.parse_args(argc, argv);
	}
	catch (const std::exception& err) {
		for (int i = 0; i < argc; std::cerr << argv[i++]<<" ");
		std::cerr << std::endl;
		std::cerr << err.what() << std::endl;
		std::cerr << program;
		std::exit(1);
	}

	boolean dryRun = program.get<bool>("--dryrun");
	boolean verbose = program.get<bool>("--verbose");
	std::string installationPath = program.get("installation");
	std::optional<std::string> dllPathOpt = program.present("--dll");
	std::optional<std::string> exePathOpt = program.present("--exe");
	
	std::optional<std::string> dllPath = dllPathOpt.has_value() ? dllPathOpt : search_file(installationPath, "chrome.dll");
	std::optional<std::string> exePath = exePathOpt.has_value() ? exePathOpt : search_file(installationPath, "chrome.exe");

	std::vector<std::string> passargStrV = program.get<std::vector<std::string>>("--args");
	for (int i = 0; i < passargStrV.size(); std::cout << passargStrV[i++] << " ");


	if (!dllPath.has_value() || !exePath.has_value()) {
		std::cerr << "chrome.dll or chrome.exe not found";
		exit(1);
	}


	UINT64 breakpointOffset = scanTgtBreakPointOffset(dllPath.value().c_str());


	
	std::string CmdLine;
	passargStrV.insert(passargStrV.begin(), exePath.value());
	boolean cmdValid= StrVGetDebuggeeCommandLine(passargStrV, 0, CmdLine);
	if (!cmdValid || CmdLine.empty())
	{
		// invalid command line
		std::cerr << "Invalid command line arguments for debugee: "<< CmdLine;
		return 1;
	}

	if (program["--dryrun"] == true) {
		std::cout << "Command: " << CmdLine << std::endl;
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
	std::cout << "Command line: " << CmdLine << "\n\n";
	if (!StartDebugeeProcess(CmdLine, g_client))
	{
		std::cerr << "StartDebugeeProcess() failed." << std::endl;
		return 0;
	}
	

	ChromeDBGCallback myCallback(g_client, breakpointOffset);

	g_client5->SetEventCallbacks(&myCallback);
	
	if (!verbose) {
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
// Build Command Line for debugee. Modified to use c++ strings.
bool StrVGetDebuggeeCommandLine(std::vector<std::string> args, int StartIndex, std::string& CmdLine)
{
	// Cleanup the [out] parameter
	CmdLine = "";
	size_t argc = args.size();

	if ((argc <= 0) || (StartIndex < 0) || (StartIndex >= argc))
	{
		return false;
	}

	while (StartIndex < argc)
	{
		CmdLine += args[StartIndex];
		if (StartIndex < argc)
			CmdLine += " ";

		StartIndex++;
	}

	// Complete 

	return true;
}

// TODO: move this somewhere else
std::optional<std::string> search_file(const std::filesystem::path& folder_path, const std::string& target_filename) {
	try {
		if (!std::filesystem::exists(folder_path) || !std::filesystem::is_directory(folder_path)) {
			std::cout << "Invalid directory path.\n";
			exit(1);
		}

		auto iter = std::filesystem::recursive_directory_iterator(folder_path);
		for (const auto& entry : iter) {
			if (iter.depth() >= SEARCH_DEPTH) {
				iter.disable_recursion_pending();
			}
			if (std::filesystem::is_regular_file(entry) && entry.path().filename() == target_filename) {
				return entry.path().string();
			}
		}
	}
	catch (const std::filesystem::filesystem_error& e) {
		std::cerr << "Error: " << e.what() << "\n";
		exit(1);
	}
	return std::nullopt;
}
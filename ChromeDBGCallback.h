///////////////////////////////////////////////////////////////////////////////
// 
// Author: Wong Yiu Kwan, Law Chun Kit
// 
// Debugger Callback Handler Class for Google Chrome MV2 Stage overwriting
// 
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include <windows.h>

#include <stdio.h>

#include <string>
#include <tchar.h>

#include "BaseConfig.h"

// TODO: set this dynamically
// Offset to OnExtensionSystemReady() method in Loaded DLL 
// Calculation:  [fileOffset] - [SizeOfHeaders] + [.text Section Address]
//#define CHROME_ExSysRdy_OFFSET 0x144DDAE

enum TgtBreakPointEnum
{
	CHROME_OnExtensionSystemReady
};

class ChromeDBGCallback : public DebugBaseEventCallbacks {
private:
	ULONG refCount = 0;
	ULONG64 chromeDllOffset = 0;
	UINT64 tgtBreakpointOffset;
	bool bpNonce = true;

	UINT32 INTEREST_MASK = 
		DEBUG_EVENT_BREAKPOINT | 
		DEBUG_EVENT_LOAD_MODULE | 
		DEBUG_EVENT_EXIT_PROCESS
	;

	// Handle chrome.dll load event
	// Adds breakpoint at CHROME_ExSysRdy_OFFSET with id CHROME_OnExtensionSystemReady
	void HandleDLL_ChromeDLL(
		_In_ ULONG64 BaseOffset,
		_In_ PCSTR ImageName
	);

	// Handle OnExtensionSystemReady breakpoint
	// Overwrites MV2Stage to 0
	void HandleBP_ExSysRdy(PDEBUG_BREAKPOINT Bp);

public:
	bool listening = true;
	PDEBUG_CLIENT g_client = nullptr;

	ChromeDBGCallback(PDEBUG_CLIENT g_client, UINT64 tgtBreakpointOffset);

	ULONG AddRef();

	ULONG Release();

	STDMETHOD(LoadModule)(
		THIS_
		_In_ ULONG64 ImageFileHandle,
		_In_ ULONG64 BaseOffset,
		_In_ ULONG ModuleSize,
		_In_ PCSTR ModuleName,
		_In_ PCSTR ImageName,
		_In_ ULONG CheckSum,
		_In_ ULONG TimeDateStamp
		);

	STDMETHOD(Breakpoint)(
		THIS_
		_In_ PDEBUG_BREAKPOINT Bp
		);

	STDMETHOD(ExitProcess)(
		THIS_
		_In_ ULONG ExitCode
		);


	HRESULT GetInterestMask(PULONG Mask);
};


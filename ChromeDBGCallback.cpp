#include "ChromeDBGCallback.h"

ChromeDBGCallback::ChromeDBGCallback(PDEBUG_CLIENT g_client, UINT64 tgtBreakpointOffset) {
	this->g_client = g_client;
	this->tgtBreakpointOffset = tgtBreakpointOffset;
}

void ChromeDBGCallback::HandleDLL_ChromeDLL(
	_In_ ULONG64 BaseOffset,
	_In_ PCSTR ImageName
) {
	if (bpNonce) {
		bpNonce = false;
		printf("Module Loaded:    %s \n\n", ImageName);
		printf("DLL Offset:    %llx \n\n", BaseOffset);
		ULONG64 bp_offset = BaseOffset + this->tgtBreakpointOffset;
		ULONG64 buf = 0;

		IDebugControl3* dbg_control_3 = nullptr;
		g_client->QueryInterface(__uuidof(IDebugControl3), (void**)&dbg_control_3);

		IDebugDataSpaces4* dbg_dataspace_4 = nullptr;
		g_client->QueryInterface(__uuidof(IDebugDataSpaces4), (void**)&dbg_dataspace_4);
		dbg_dataspace_4->ReadVirtual(bp_offset, &buf, sizeof(ULONG64), NULL);

		printf("OPCODE addr :    %llx \n\n", bp_offset);
		printf("test read OPCODE:    %llx \n\n", buf);

		PDEBUG_BREAKPOINT bp;
		dbg_control_3->AddBreakpoint(DEBUG_BREAKPOINT_CODE, CHROME_OnExtensionSystemReady, &bp);
		bp->SetOffset(bp_offset);
		bp->AddFlags(DEBUG_BREAKPOINT_ENABLED);
	}
}

void ChromeDBGCallback::HandleBP_ExSysRdy(PDEBUG_BREAKPOINT Bp) {
	// Get the interface for accessing process registers
	IDebugRegisters* regs = nullptr;
	g_client->QueryInterface(__uuidof(IDebugRegisters), (void**)&regs);


	ULONG r8d_index;
	regs->GetIndexByName("r8d", &r8d_index);
	DEBUG_VALUE r8d_val;
	regs->GetValue(r8d_index, &r8d_val);


	// Get the interface for accessing virtual memory
	IDebugDataSpaces4* dbg_dataspace_4 = nullptr;
	g_client->QueryInterface(__uuidof(IDebugDataSpaces4), (void**)&dbg_dataspace_4);

	// Test if manifest version is valid range
	if (r8d_val.I32 > 3) {
		// something wrong
		_tprintf(_T("Unexpected manifest version. Abort overwriting\n"));
		_tprintf(_T("RSI:    %lx\n\n"), r8d_val.I32);
		return;
	}

	r8d_val.I32 = 3;
	r8d_val.Type = DEBUG_VALUE_INT32;
	regs->SetValue(r8d_index, &r8d_val);
}


HRESULT ChromeDBGCallback::LoadModule(
	THIS_
	_In_ ULONG64 ImageFileHandle,
	_In_ ULONG64 BaseOffset,
	_In_ ULONG ModuleSize,
	_In_ PCSTR ModuleName,
	_In_ PCSTR ImageName,
	_In_ ULONG CheckSum,
	_In_ ULONG TimeDateStamp
)
{

	if (strncmp(ModuleName, "chrome", 7) == 0) {
		//if (EndsWith(ImageName, "chrome.dll")) {
		HandleDLL_ChromeDLL(BaseOffset, ImageName);
	}


	UNREFERENCED_PARAMETER(ImageFileHandle);
	//UNREFERENCED_PARAMETER(BaseOffset);
	UNREFERENCED_PARAMETER(ModuleSize);
	//UNREFERENCED_PARAMETER(ModuleName);
	//UNREFERENCED_PARAMETER(ImageName);
	UNREFERENCED_PARAMETER(CheckSum);
	UNREFERENCED_PARAMETER(TimeDateStamp);
	return DEBUG_STATUS_NO_CHANGE;
}

HRESULT ChromeDBGCallback::Breakpoint(
	THIS_
	_In_ PDEBUG_BREAKPOINT Bp
)
{
	ULONG bpId = 0;
	Bp->GetId(&bpId);

	switch (bpId) {
	case CHROME_OnExtensionSystemReady:
		HandleBP_ExSysRdy(Bp);
		break;

	default:
		break;
	}

	return DEBUG_STATUS_NO_CHANGE;
}

HRESULT ChromeDBGCallback::ExitProcess(
	THIS_
	_In_ ULONG ExitCode
)
{
	listening = false;
	_tprintf(_T("Debug Target exited with code: %ul\n"), ExitCode);
	return DEBUG_STATUS_NO_CHANGE;
}


HRESULT ChromeDBGCallback::GetInterestMask(PULONG Mask) {
	*Mask = INTEREST_MASK;
	return S_OK;
}

ULONG ChromeDBGCallback::AddRef() {
	refCount++;
	return refCount;
}

ULONG ChromeDBGCallback::Release() {
	refCount--;
	return refCount;
}
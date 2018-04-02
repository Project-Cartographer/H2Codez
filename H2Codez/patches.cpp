#include "stdafx.h"
#include "Patches.h"

void WriteBytes(DWORD destAddress, LPVOID patch, DWORD numBytes)
{
	DWORD OldProtection;
	void *target_addr = reinterpret_cast<void*>(destAddress);

	VirtualProtect(target_addr, numBytes, PAGE_EXECUTE_READWRITE, &OldProtection);
	memcpy(target_addr, patch, numBytes);
	VirtualProtect(target_addr, numBytes, OldProtection, NULL);

	FlushInstructionCache(GetCurrentProcess(), target_addr, numBytes);
}

void PatchCall(DWORD call_addr, DWORD new_function_ptr) {
	DWORD callRelative = new_function_ptr - (call_addr + 5);
	WritePointer(call_addr + 1, reinterpret_cast<void*>(callRelative));
}

void WritePointer(DWORD offset, void *ptr) {
	BYTE* pbyte = (BYTE*)&ptr;
	BYTE assmNewFuncRel[0x4] = { pbyte[0], pbyte[1], pbyte[2], pbyte[3] };
	WriteBytes(offset, assmNewFuncRel, 0x4);
}

void PatchWinAPICall(DWORD call_addr, DWORD new_function_ptr)
{
	BYTE call = 0xE8;
	WriteValue(call_addr, call);

	PatchCall(call_addr, new_function_ptr);

	// pad the extra unused byte
	BYTE padding = 0x90;
	WriteValue(call_addr + 5, padding);
}

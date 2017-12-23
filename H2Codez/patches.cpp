#include "stdafx.h"
#include "Patches.h"

VOID WriteBytes(DWORD destAddress, LPVOID patch, DWORD numBytes)
{
	DWORD OldProtection;

	VirtualProtect((LPVOID)destAddress, numBytes, PAGE_EXECUTE_READWRITE, &OldProtection);
	memcpy((LPVOID)destAddress, patch, numBytes);
	VirtualProtect((LPVOID)destAddress, numBytes, OldProtection, NULL);
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

#pragma once
#include "stdafx.h"



VOID WriteBytesASM(DWORD destAddress, LPVOID patch, DWORD numBytes)
{
	DWORD oldProtect = 0;
	DWORD srcAddress = PtrToUlong(patch);

	VirtualProtect((void*)(destAddress), numBytes, PAGE_EXECUTE_READWRITE, &oldProtect);

	__asm
	{
		nop
		nop
			nop

			mov esi, srcAddress
			mov edi, destAddress
			mov ecx, numBytes
			Start :
		cmp ecx, 0
			jz Exit

			mov al, [esi]
			mov[edi], al
			dec ecx
			inc esi
			inc edi
			jmp Start
			Exit :
		nop
			nop
			nop
	}

	VirtualProtect((void*)(destAddress), numBytes, oldProtect, &oldProtect);
}
VOID PatchCall(DWORD call_addr, DWORD new_function_ptr) 
{
	DWORD callRelative = new_function_ptr - (call_addr + 5);
	BYTE* pbyte = (BYTE*)&callRelative;
	BYTE assmNewFuncRel[4] = { pbyte[0], pbyte[1], pbyte[2], pbyte[3] };
	WriteBytesASM(call_addr + 1, assmNewFuncRel, 4);
}



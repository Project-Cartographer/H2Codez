#include "stdafx.h"
#include "Patches.h"

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

void PatchCall(DWORD call_addr, DWORD new_function_ptr) {
	DWORD callRelative = new_function_ptr - (call_addr + 5);
	WritePointer(call_addr + 1, reinterpret_cast<void*>(callRelative));
}

void WritePointer(DWORD offset, void *ptr) {
	BYTE* pbyte = (BYTE*)&ptr;
	BYTE assmNewFuncRel[0x4] = { pbyte[0], pbyte[1], pbyte[2], pbyte[3] };
	WriteBytesASM(offset, assmNewFuncRel, 0x4);
}

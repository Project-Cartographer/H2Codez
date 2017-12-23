#pragma once

VOID WriteBytes(DWORD destAddress, LPVOID patch, DWORD numBytes);
VOID PatchCall(DWORD call_addr, DWORD new_function_ptr);
VOID WritePointer(DWORD offset, void *ptr);
inline void PatchCall(DWORD call_addr, void *new_function_ptr)
{
	PatchCall(call_addr, reinterpret_cast<DWORD>(new_function_ptr));
}

#define J(symbol1, symbol2) _DO_JOIN(symbol1, symbol2)
#define _DO_JOIN(symbol1, symbol2) symbol1##symbol2

#define NopFill(Address, len)                         \
BYTE J(NopFIll_, __LINE__ )[len];                     \
	std::fill_n(J(NopFIll_, __LINE__ ), len, 0x90);   \
	WriteBytes(Address, J(NopFIll_, __LINE__ ), len)

//Write a jmp to addy at Line
#define WriteJmpTo(PatchLine,JMPtoAddr)\
    BYTE patch[1] = { 0xE8 };\
    WriteBytes((DWORD)PatchLine, patch, 1);\
    PatchCall((DWORD)PatchLine,(DWORD)JMPtoAddr);
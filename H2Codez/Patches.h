#pragma once

VOID WriteBytesASM(DWORD destAddress, LPVOID patch, DWORD numBytes);
VOID PatchCall(DWORD call_addr, DWORD new_function_ptr);

#define J(symbol1, symbol2) _DO_JOIN(symbol1, symbol2)
#define _DO_JOIN(symbol1, symbol2) symbol1##symbol2

#define NopFill(Address, len)                         \
BYTE J(NopFIll_, __LINE__ )[len];                     \
	std::fill_n(J(NopFIll_, __LINE__ ), len, 0x90);   \
	WriteBytesASM(Address, J(NopFIll_, __LINE__ ), len)

//Write a jmp to addy at Line
#define WriteJmpTo(PatchLine,JMPtoAddr)\
    BYTE patch[1] = { 0xE8 };\
    WriteBytesASM((DWORD)PatchLine, patch, 1);\
    PatchCall((DWORD)PatchLine,(DWORD)JMPtoAddr);
#include "stdafx.h"

char* wstring_to_string(char* string, int string_length, wcstring wide, int wide_length)
{
	if (!WIN32_FUNC(WideCharToMultiByte)(CP_ACP, 0, wide, wide_length, string, string_length, nullptr, nullptr))
		return nullptr;
	else
		return string;
}


DWORD H2EK_Globals::GetBase()
{
	return (DWORD)base;
}

BYTE* reverse_addr(void* address)
{
	DWORD c = (DWORD)address;
	BYTE *f = (BYTE*)(&c);
	return f;
}
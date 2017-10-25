#include "stdafx.h"
#include "H2Guerilla.h"
#include "H2Sapien.h"
#include "H2ToolsCommon.h"

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

H2EK H2Toolz::detect_type()
{
	if (GetModuleHandleW(L"h2Guerilla.exe"))
	{
		game.base = GetModuleHandleW(L"h2Guerilla.exe");
		return H2EK::H2Guerilla;
	}
	else if (GetModuleHandleW(L"h2Sapien.exe"))
	{
		game.base = GetModuleHandleW(L"h2Sapien.exe");
		return H2EK::H2Sapien;
	}
	else if (GetModuleHandleW(L"h2Tool.exe"))
	{
		game.base = GetModuleHandleW(L"h2Tool.exe");
		return H2EK::H2Tool;
	}
	return H2EK::Invalid;
}

bool H2Toolz::Init()
{
	H2CommonPatches::Init();
	game.process_type = detect_type();

	switch (game.process_type) {
	case H2EK::H2Tool:
	{
		H2Tool_Extras *tool = new H2Tool_Extras();
		tool->Initialize();
		return true;
	}
	case H2EK::H2Guerilla:
	{
		H2GuerrilaPatches::Init();
		return true;
	}
	case H2EK::H2Sapien:
	{
		H2SapienPatches::Init();
		return true;
	}
	case H2EK::Invalid:
		MessageBoxA(0, "H2toolz loaded into unsupported process, will now exit!", "ERROR!", MB_OK);
		return false;
	}
}
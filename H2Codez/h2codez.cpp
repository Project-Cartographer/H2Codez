#include "stdafx.h"
#include "H2Guerilla.h"
#include "H2Sapien.h"
#include "H2ToolsCommon.h"
#include "DiscordInterface.h"

char app_directory[256];
std::wstring_convert<std::codecvt_utf8<wchar_t>> wstring_to_string;

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
	GetCurrentDirectory(sizeof(app_directory), app_directory);
	game.process_type = detect_type();
	DiscordInterface::setAppType(game.process_type);
	H2CommonPatches::Init();

	switch (game.process_type) {
	case H2EK::H2Tool:
	{
		H2ToolPatches::Initialize();
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
	abort(); // should be unreachable
}
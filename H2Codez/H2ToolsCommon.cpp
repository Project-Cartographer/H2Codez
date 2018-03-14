#include <cwchar>
#include "stdafx.h"
#include "Patches.h"
#include "H2ToolsCommon.h"
#include <regex>
#include "Psapi.h"
#include "DiscordInterface.h"
#include "Debug.h"
#include <cassert>
#include <Shellapi.h>
#include "HaloScriptCommon.h"

using namespace HaloScriptCommon;

typedef int (WINAPI *LoadStringW_Typedef)(HINSTANCE hInstance, UINT uID, LPWSTR lpBuffer, int cchBufferMax);
LoadStringW_Typedef LoadStringW_Orginal;

typedef wchar_t* (WINAPI *GetCommandLineW_Typedef)();
GetCommandLineW_Typedef GetCommandLineW_Orginal;

static const wchar_t *map_types[] = 
{
	L"Single Player",
	L"Multiplayer",
	L"Main Menu",
	L"Multiplayer Shared",
	L"Single Player Shared"
};

static wchar_t const open_as_text[] = L"Export as text";

int WINAPI LoadStringW_Hook(HINSTANCE hInstance, UINT uID, LPWSTR lpBuffer, int cchBufferMax)
{ 
	if (GetModuleHandleW(L"H2alang") != hInstance)
		return LoadStringW_Orginal(hInstance, uID, lpBuffer, cchBufferMax);
	if (310 <= uID && uID <= 318) {
		wcsncpy_s(lpBuffer, cchBufferMax, map_types[uID / 2 - 155], cchBufferMax);
		return std::wcslen(lpBuffer);
	}
	if (uID == 26)
	{
		wcsncpy_s(lpBuffer, cchBufferMax, open_as_text, sizeof(open_as_text));
		return std::wcslen(lpBuffer);
	}
	return LoadStringW_Orginal(hInstance, uID, lpBuffer, cchBufferMax);
}


bool discord_init_finished = false;
wchar_t* __stdcall GetCommandLineW_Hook()
{
	if (!discord_init_finished) {
		DiscordInterface::Init();
		discord_init_finished = true;
	}
	wchar_t *real_cmd = GetCommandLineW_Orginal();
	std::wstring fake_cmd = std::regex_replace(real_cmd, std::wregex(L"( pause_after_run| shared_tag_removal)"), L"");
	wcscpy(real_cmd, fake_cmd.c_str());
	return real_cmd;
}

bool H2CommonPatches::newInstance()
{
	TCHAR exePath[MAX_PATH];
	if (!GetModuleFileName(game.base, exePath, MAX_PATH))
		return false;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	CreateProcess(exePath, nullptr, nullptr, nullptr, false, INHERIT_PARENT_AFFINITY, nullptr, nullptr, &si, &pi);
	return true;
}

std::string get_command_usage_by_id(unsigned short id)
{
	char usage_string[0x800];
	int get_command_usage_by_id_impl = SwitchAddessByMode(0, 0x4E2DF0, 0x4D4F90);
	CHECK_FUNCTION_SUPPORT(get_command_usage_by_id_impl);

	__asm {
		mov ax, id
		lea edi, usage_string
		mov esi, 0x800
		call get_command_usage_by_id_impl
	}
	return usage_string;
}

std::string H2CommonPatches::get_temp_name(std::string name_suffix)
{
	std::string name = std::tmpnam(nullptr);
	if (name_suffix.size() > 0)
		name += "." + name_suffix;
	return name;
}

void H2CommonPatches::copy_to_clipboard(std::string text, HWND owner)
{
	if (OpenClipboard(owner)) {
		EmptyClipboard();

		size_t len = text.length() + 1;
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
		if (hMem != NULL) {
			char *clipboard_text = static_cast<char*>(GlobalLock(hMem));
			strncpy(clipboard_text, text.c_str(), len);
			GlobalUnlock(hMem);

			SetClipboardData(CF_TEXT, hMem);
		}
		CloseClipboard();
	}
}

bool H2CommonPatches::read_clipboard(std::string &contents, HWND owner)
{
	if (OpenClipboard(owner)) {
		HANDLE data = GetClipboardData(CF_TEXT);
		if (data != NULL) {
			LPTSTR text = static_cast<LPTSTR>(GlobalLock(data));
			if (text != NULL) {
				contents = text;
				GlobalUnlock(data);
				return true;
			}
		}
		CloseClipboard();
	}
	return false;
}

void H2CommonPatches::generate_script_doc(const char *filename)
{
	FILE *FilePtr;

	int command_table_ptr_offset = SwitchAddessByMode(0, 0x9E9E90, 0x95BF70);
	int global_table_ptr_offset = SwitchAddessByMode(0, 0x9ECE28, 0x95EF08);
	CHECK_FUNCTION_SUPPORT(global_table_ptr_offset);

	std::string file_name = get_temp_name("hs_doc.txt");
	if (filename)
		file_name = filename;
	hs_command **command_table = reinterpret_cast<hs_command **>(command_table_ptr_offset);
	hs_global_variable **global_table = reinterpret_cast<hs_global_variable **>(global_table_ptr_offset);

	if (!fopen_s(&FilePtr, file_name.c_str(), "w"))
	{	
		fprintf(FilePtr, "== Commands ==\r\n\r\n");
		for (USHORT current_command_id = 0; current_command_id < 924; current_command_id++)
		{
			fprintf(FilePtr, "%s\r\n", get_command_usage_by_id(current_command_id).c_str());
			fprintf(FilePtr, "%s\r\n\r\n", command_table[current_command_id]->desc);
		}
		fprintf(FilePtr, "== Script Globals ==\r\n\r\n");
		for (USHORT current_global_id = 0; current_global_id < 706; current_global_id++)
		{
			hs_global_variable *current_var = global_table[current_global_id];
			fprintf(FilePtr, "(%s <%s>)\r\n", current_var->name, hs_type_string[current_var->type].c_str());
		}
		fclose(FilePtr);
	}
	ShellExecuteA(NULL, NULL, file_name.c_str(), NULL, NULL, SW_SHOW);
}

void H2CommonPatches::Init()
{
	Debug::init();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	LoadStringW_Orginal = LoadStringW;
	DetourAttach(&(PVOID&)LoadStringW_Orginal, LoadStringW_Hook);

	GetCommandLineW_Orginal = GetCommandLineW;
	DetourAttach(&(PVOID&)GetCommandLineW_Orginal, GetCommandLineW_Hook);

	DetourTransactionCommit();
}
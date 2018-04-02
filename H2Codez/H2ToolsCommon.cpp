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

typedef void (WINAPI *ExitProcess_Typedef)(UINT exitcode);
ExitProcess_Typedef ExitProcess_Orginal;

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
		DiscordInterface::init();
		discord_init_finished = true;
	}
	wchar_t *real_cmd = GetCommandLineW_Orginal();
	std::wstring fake_cmd = std::regex_replace(real_cmd, std::wregex(L"( pause_after_run| shared_tag_removal)"), L"");
	wcscpy(real_cmd, fake_cmd.c_str());
	return real_cmd;
}

void __stdcall ExitProcess_Hook(UINT exitcode)
{
	DiscordInterface::shutdown();
	return ExitProcess_Orginal(exitcode);
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

std::string get_command_usage(hs_command *cmd)
{
	std::string usage = "(<" + hs_type_string[cmd->return_type] + "> " + cmd->name;
	if (cmd->usage) {
		usage += std::string(" ") + cmd->usage;
	} else {
		for (size_t arg = 0; arg < cmd->arg_count; arg++)
		{
			hs_type arg_type = static_cast<hs_type>(cmd->arg_type_array[arg]);
			usage += " <" + hs_type_string[arg_type] + ">";
		}
	}
	usage += ")";
	return usage;
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
			hs_command *cmd = command_table[current_command_id];
			fprintf(FilePtr, "%s\r\n", get_command_usage(cmd).c_str());
			fprintf(FilePtr, "%s\r\n\r\n", cmd->desc);
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

typedef char(__cdecl *get_halo_2_documents_path)(LPWSTR pszPath);
int get_h2_path_offset = SwitchAddessByMode(0x00589C30, 0x004BA6E0, 0x00489F70);
get_halo_2_documents_path get_halo_2_documents_path_impl = reinterpret_cast<get_halo_2_documents_path>(get_h2_path_offset);

char narrow_path[0x200];
// The toolkit seems to misuse this function so much that it's easier to replace it,
// with a working one then fix the code that misuses it.
char* __stdcall get_narrow_halo_2_documents_path()
{
	wchar_t wide_path[0x200];

	get_halo_2_documents_path_impl(wide_path);
	WideCharToMultiByte(CP_UTF8, 0, wide_path, -1, narrow_path, sizeof(narrow_path), NULL, NULL);

	return narrow_path;
}

wchar_t wide_path[0x200];
wchar_t* __stdcall get_wide_halo_2_documents_path()
{
	get_halo_2_documents_path_impl(wide_path);
	return wide_path;
}

// The toolkit was treating a wide string as a narrow one in a lot of FS related functions
// This fixes that by making sure the code gets the string type it was expecting
void fix_documents_path_string_type()
{
	WriteJmpTo(SwitchAddessByMode(0x00589D10, 0x004BA7C0, 0x0048A050), get_narrow_halo_2_documents_path);

	// The only two functions that weren't broken before
	PatchCall(SwitchAddessByMode(0x006708E6, 0x005061A5, 0x005AEFF6), get_wide_halo_2_documents_path); // wrl export
	PatchCall(SwitchAddessByMode(0x00670B05, 0x005061C2, 0x005AF215), get_wide_halo_2_documents_path); // comments export
	if (game.process_type == H2Tool) {
		PatchCall(0x00430E76, get_wide_halo_2_documents_path); // working without patches
		PatchCall(0x00445FDA, get_wide_halo_2_documents_path); // not sure if working or broken, leaving as is for now
	}
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

	ExitProcess_Orginal = ExitProcess;
	DetourAttach(&(PVOID&)ExitProcess_Orginal, ExitProcess_Hook);

	fix_documents_path_string_type();

	DetourTransactionCommit();
}
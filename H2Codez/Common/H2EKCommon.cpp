#include "H2EKCommon.h"
#include "../stdafx.h"
#include "../util/Patches.h"
#include "Psapi.h"
#include "DiscordInterface.h"
#include "../util/Debug.h"
#include "../HaloScript/hs_interface.h"
#include <cwchar>
#include <cassert>
#include <Shellapi.h>
#include <Shlwapi.h>
#include <Shlobj.h>
#include <CommDlg.h>

using namespace H2CommonPatches;

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
static wchar_t const scenario_saved[] = L"%s.%hs saved";

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
	if (uID == 0x1018)
	{
		wcsncpy_s(lpBuffer, cchBufferMax, scenario_saved, sizeof(scenario_saved));
		return std::wcslen(lpBuffer);
	}
	return LoadStringW_Orginal(hInstance, uID, lpBuffer, cchBufferMax);
}

bool discord_init_finished = false;
wchar_t* __stdcall GetCommandLineW_Hook()
{
	if (!discord_init_finished &&
			conf.getBoolean("patches_enabled", true) && conf.getBoolean("discord_enabled", true)) {
		DiscordInterface::init();
		discord_init_finished = true;
	}
	return GetCommandLineW_Orginal();
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

std::wstring H2CommonPatches::GetExeDirectory()
{
	wchar_t main_exe_dir[MAX_PATH];
	GetModuleFileNameW(NULL, main_exe_dir, sizeof(main_exe_dir));
	size_t path_len = wcsnlen_s(main_exe_dir, sizeof(main_exe_dir));
	if (main_exe_dir[path_len - 1] == L'\\')
		main_exe_dir[path_len - 1] = NULL;
	wchar_t *last_part = wcsrchr(main_exe_dir, L'\\');
	if (last_part)
		*last_part = NULL;
	return main_exe_dir;
}

std::string H2CommonPatches::get_temp_name(const std::string &name_suffix)
{
	std::string name = std::tmpnam(nullptr);
	if (name_suffix.size() > 0)
		name += "." + name_suffix;
	return name;
}

bool H2CommonPatches::copy_to_clipboard(const std::string &text, HWND owner)
{
	bool success = false;
	if (LOG_CHECK(OpenClipboard(owner))) {
		if (owner != NULL)
			LOG_CHECK(EmptyClipboard() != FALSE);

		size_t len = text.length() + 1;
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
		if (LOG_CHECK(hMem != NULL)) {
			char *clipboard_text = LOG_CHECK(static_cast<char*>(GlobalLock(hMem)));
			if (clipboard_text) {
				strncpy(clipboard_text, text.c_str(), len);
				if (LOG_CHECK(GlobalUnlock(hMem) || GetLastError() == NO_ERROR))
					success = (SetClipboardData(CF_TEXT, hMem) != NULL);
			}
		}
		LOG_CHECK(CloseClipboard());
	}
	return success;
}

bool H2CommonPatches::read_clipboard(std::string &contents, HWND owner)
{
	if (LOG_CHECK(OpenClipboard(owner))) {
		HANDLE data = GetClipboardData(CF_TEXT);
		if (LOG_CHECK(data != NULL)) {
			LPTSTR text = static_cast<LPTSTR>(GlobalLock(data));
			if (LOG_CHECK(text != NULL)) {
				contents = text;
				if (LOG_CHECK(GlobalUnlock(data) || GetLastError() == NO_ERROR))
					return true;
			}
		}
		LOG_CHECK(CloseClipboard());
	}
	return false;
}

void H2CommonPatches::generate_script_doc(const char *filename)
{
	FILE *FilePtr;

	std::string file_name = get_temp_name("hs_doc.txt");
	if (filename)
		file_name = filename;

	if (!fopen_s(&FilePtr, file_name.c_str(), "w"))
	{	
		fprintf(FilePtr, "== Commands ==\r\n\r\n");
		for (hs_command *cmd : g_halo_script_interface->command_table)
		{
			fprintf(FilePtr, "%s\r\n", get_command_usage(cmd).c_str());
			fprintf(FilePtr, "%s\r\n\r\n", cmd->desc);
		}

		fprintf(FilePtr, "== Script Globals ==\r\n\r\n");
		for (hs_global_variable *current_var : g_halo_script_interface->global_table)
		{
			fprintf(FilePtr, "(%s <%s>)\r\n", current_var->name, hs_type_string[current_var->type].c_str());
		}
		fclose(FilePtr);
	}
	ShellExecuteA(NULL, NULL, file_name.c_str(), NULL, NULL, SW_SHOW);
}

char narrow_path[0x200];
// The toolkit seems to misuse this function so much that it's easier to replace it,
// with a working one then fix the code that misuses it.
char* __stdcall get_narrow_halo_2_documents_path()
{
	SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, narrow_path);
	PathAppendA(narrow_path, "Halo 2");
	SHCreateDirectoryEx(0, narrow_path, 0);

	return narrow_path;
}

wchar_t wide_path[0x200];
wchar_t* __stdcall get_wide_halo_2_documents_path()
{
	SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, wide_path);
	PathAppendW(wide_path, L"Halo 2");
	SHCreateDirectoryExW(0, wide_path, 0);
	return wide_path;
}

// The toolkit was treating a wide string as a narrow one in a lot of FS related functions
// This fixes that by making sure the code gets the string type it was expecting
void fix_documents_path_string_type()
{
	WriteJmp(SwitchAddessByMode(0x00589D10, 0x004BA7C0, 0x0048A050), get_narrow_halo_2_documents_path);

	// The only two functions that weren't broken before
	PatchCall(SwitchAddessByMode(0x006708E6, 0x005061A5, 0x005AEFF6), get_wide_halo_2_documents_path); // wrl export
	PatchCall(SwitchAddessByMode(0x00670B05, 0x005061C2, 0x005AF215), get_wide_halo_2_documents_path); // comments export
	if (game.process_type == H2Guerilla) {
		PatchCall(0x00430E76, get_wide_halo_2_documents_path); // working without patches
		PatchCall(0x00445FDA, get_wide_halo_2_documents_path); // not sure if working or broken, leaving as is for now
	}
}

void **__cdecl halo_2_only_stub(int opcode, void *DatumIndex, char user_cmd)
{
	return HaloScriptCommon::epilog(DatumIndex, 0);
}

hs_command *unknown_command = NewCommand
(
	"unknown_command",
	hs_type::nothing,
	hs_default_func_check,
	halo_2_only_stub,
	"Does nothing."
);

int api_version = 1;

hs_global_variable api_extension_version = hs_global_variable
(
	"api_extension_version",
	hs_type::hs_long,
	&api_version
);

void HaloScriptExtensions()
{
	// start halo nops
	g_halo_script_interface->RegisterCommand(hs_opcode::hs_unk_1, unknown_command);
	g_halo_script_interface->RegisterCommand(hs_opcode::hs_unk_2, unknown_command);
	g_halo_script_interface->RegisterCommand(hs_opcode::hs_unk_3, unknown_command);
	g_halo_script_interface->RegisterCommand(hs_opcode::hs_unk_4, unknown_command);
	// end halo nops

	g_halo_script_interface->RegisterGlobal(hs_global_id::api_extension_version, &api_extension_version);
}

typedef BOOL (WINAPI *T_FuncOpenFileNameW)(LPOPENFILENAMEW info);
T_FuncOpenFileNameW GetOpenFileNameWOriginal;
BOOL WINAPI GetOpenFileNameWHook(LPOPENFILENAMEW info)
{
	// check if it's the basic file select dialog
	if (reinterpret_cast<DWORD>(info->lpfnHook) == SwitchAddessByMode(0, 0x0040c450, 0x0069a0f7))
		info->Flags &= ~OFN_ENABLEHOOK; //  disable hook, and use default windows syle
	return GetOpenFileNameWOriginal(info);
}

T_FuncOpenFileNameW GetSaveFileNameWOriginal;
BOOL WINAPI GetSaveFileNameWHook(LPOPENFILENAMEW info)
{
	// check if it's the basic file select dialog
	if (reinterpret_cast<DWORD>(info->lpfnHook) == SwitchAddessByMode(0, 0x0040c450, 0x0069a0f7))
		info->Flags &= ~OFN_ENABLEHOOK; //  disable hook, and use default windows syle
	return GetSaveFileNameWOriginal(info);
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

	// disable custom save/open dialogs 

	GetOpenFileNameWOriginal = GetOpenFileNameW;
	DetourAttach(&(PVOID&)GetOpenFileNameWOriginal, GetOpenFileNameWHook);

	GetSaveFileNameWOriginal = GetSaveFileNameW;
	DetourAttach(&(PVOID&)GetSaveFileNameWOriginal, GetSaveFileNameWHook);

	fix_documents_path_string_type();

	HaloScriptExtensions();

	DetourTransactionCommit();
}
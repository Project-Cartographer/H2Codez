#include "H2EKCommon.h"
#include "stdafx.h"
#include "util/Patches.h"
#include "Psapi.h"
#include "DiscordInterface.h"
#include "util/Debug.h"
#include "HaloScript/hs_interface.h"
#include <cwchar>
#include <cassert>
#include <Shellapi.h>
#include <Shlwapi.h>
#include <Shlobj.h>
#include <CommDlg.h>
#include <mutex>
#include "util/crc32.h"

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

int WINAPI LoadStringW_Hook(HINSTANCE hInstance, UINT uID, LPWSTR lpBuffer, int cchBufferMax)
{ 
	if (GetModuleHandleW(L"H2alang") != hInstance)
		return LoadStringW_Orginal(hInstance, uID, lpBuffer, cchBufferMax);
	if (310 <= uID && uID <= 318) {
		wcsncpy_s(lpBuffer, cchBufferMax, map_types[uID / 2 - 155], cchBufferMax);
		return std::wcslen(lpBuffer);
	}
	if (uID == 26) // org: open as text
	{
		wcscpy_s(lpBuffer, cchBufferMax, L"Export as text");
		return std::wcslen(lpBuffer);
	}
	if (uID == 0x1018)
	{
		wcscpy_s(lpBuffer, cchBufferMax, L"%s.%hs saved");
		return std::wcslen(lpBuffer);
	}
	if (uID == 0x12AE)
	{
		wcscpy_s(lpBuffer, cchBufferMax, L"Unit Playtest");
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

std::wstring H2CommonPatches::GetExeDirectoryWide()
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

std::string H2CommonPatches::GetExeDirectoryNarrow()
{
	char main_exe_dir[MAX_PATH];
	GetModuleFileNameA(NULL, main_exe_dir, sizeof(main_exe_dir));
	size_t path_len = strnlen_s(main_exe_dir, sizeof(main_exe_dir));
	if (main_exe_dir[path_len - 1] == L'\\')
		main_exe_dir[path_len - 1] = NULL;
	char *last_part = strrchr(main_exe_dir, L'\\');
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

std::mutex clipboard_mutex;

bool H2CommonPatches::copy_to_clipboard(const std::string &text, HWND owner)
{
	std::unique_lock<std::mutex> clipboard_lock(clipboard_mutex);
	bool success = false;
	if (LOG_CHECK(OpenClipboard(owner))) {
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
	std::unique_lock<std::mutex> clipboard_lock(clipboard_mutex);
	if (!IsClipboardFormatAvailable(CF_TEXT))
		return false;
	if (LOG_CHECK(OpenClipboard(owner))) {
		HANDLE data = GetClipboardData(CF_TEXT);
		if (LOG_CHECK(data != NULL)) {
			LPTSTR text = static_cast<LPTSTR>(GlobalLock(data));
			if (LOG_CHECK(text != NULL)) {
				contents = text;
				if (LOG_CHECK(GlobalUnlock(data) || GetLastError() == NO_ERROR))
				{
					LOG_CHECK(CloseClipboard());
					return true;
				}
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

int api_version = 1;

hs_global_variable api_extension_version = hs_global_variable
(
	"api_extension_version",
	hs_type::hs_long,
	&api_version
);

bool wake_hs_thread_by_name(char *thread)
{
	typedef char __cdecl _wake_hs_thread_by_name(char *thread);
	auto _wake_hs_thread_by_name_impl = reinterpret_cast<_wake_hs_thread_by_name*>(0x52C5B0);
	return _wake_hs_thread_by_name_impl(thread);
}

void fix_haloscript_pointers()
{
	// Replace pointers to the commmand table
	const static std::vector<DWORD> cmd_table_offsets_tool =
	{
		0x005C5365 + 3, 0x005C5530 + 3, 0x005C5554 + 3,
		0x005C5821 + 3, 0x005C5C44 + 3, 0x005C5E64 + 3,
		0x005C5E9A + 3, 0x005C5F48 + 3
	};
	const static std::vector<DWORD> cmd_table_offsets_sapien =
	{
		0x004E2225 + 3, 0x004E23F0 + 3,  0x004E2414 + 3,
		0x004E2701 + 3, 0x004E2DF4 + 3, 0x004E3014 + 3,
		0x004E304D,     0x004E30FB
	};
	std::vector<DWORD> &cmd_table_offsets = SwitchByMode(cmd_table_offsets_tool, cmd_table_offsets_sapien, {});

	hs_command **cmds = g_halo_script_interface->get_command_table();

	for (DWORD addr : cmd_table_offsets)
		WritePointer(addr, cmds);

	// patch command table size
	const static int hs_cmd_table_size = g_halo_script_interface->get_command_table_size();
	WriteValue(SwitchByMode(0x008CD59C, 0x008EB118, NULL), hs_cmd_table_size);

	// Replace pointers to the globals table

	const static std::vector<DWORD> var_table_offsets_tool =
	{
		0x005C53D5, 0x005C53F0, 0x005C5430,
		0x005C5474, 0x005C58D1, 0x006884A1,
		0x006884BD, 0x0068850D, 0x0068858B,
		0x006885A2
	};
	const static std::vector<DWORD> var_table_offsets_sapien =
	{
		0x004E2295, 0x004E22B0, 0x004E22F0,
		0x004E2334, 0x004E27B1, 0x00635A11,
		0x00635A2D, 0x00635A7D, 0x00635AFB,
		0x00635B12
	};

	std::vector<DWORD> &var_table_offsets = SwitchByMode(var_table_offsets_tool, var_table_offsets_sapien, {});

	hs_global_variable **vars = g_halo_script_interface->get_global_table();

	for (DWORD addr : var_table_offsets)
		WriteValue(addr + 3, vars);

	// patch globals table size
	const static int hs_global_table_size = g_halo_script_interface->get_global_table_size();
	WriteValue(SwitchByMode(0x008D2238, 0x008EFDB4, NULL), hs_global_table_size);
}

void init_haloscript_patches()
{
	hs_command **command_table = reinterpret_cast<hs_command **>(SwitchByMode(0x009ECFE0, 0x9E9E90, 0x95BF70));
	hs_global_variable **global_table = reinterpret_cast<hs_global_variable **>(SwitchByMode(0x009EFF78, 0x9ECE28, 0x95EF08));
	g_halo_script_interface->init_custom(command_table, global_table);

	fix_haloscript_pointers();
#pragma region unknown nops
	hs_custom_command unknown_stub(
		"unknown_command",
		"Does nothing.",
		NULL_HS_FUNC
	);
	g_halo_script_interface->RegisterCustomCommand(hs_opcode::hs_unk_1, unknown_stub);
	g_halo_script_interface->RegisterCustomCommand(hs_opcode::hs_unk_2, unknown_stub);
	g_halo_script_interface->RegisterCustomCommand(hs_opcode::hs_unk_3, unknown_stub);
	g_halo_script_interface->RegisterCustomCommand(hs_opcode::hs_unk_4, unknown_stub);
#pragma endregion

#pragma region extensions

	hs_custom_command enable_custom_script_sync("enable_custom_script_sync", "Allows running scripts on client using wake_sync (extension function).", NULL_HS_FUNC); // does nothing in sapien
	g_halo_script_interface->RegisterCustomCommand(hs_opcode::enable_custom_script_sync, enable_custom_script_sync);

	struct start_sync_args
	{
		char *script_name;
	};
	auto start_sync_func = HS_FUNC(
		wake_hs_thread_by_name((static_cast<start_sync_args*>(args))->script_name);
		return 0;
		);
	hs_custom_command wake_sync("wake_sync",
		"Run a script on both server and clients (extension function).",
		start_sync_func,
		{ hs_type::string }
	);
	g_halo_script_interface->RegisterCustomCommand(hs_opcode::wake_sync, wake_sync);

	g_halo_script_interface->RegisterGlobal(hs_global_id::api_extension_version, &api_extension_version);
#pragma endregion

	struct set_temp_args
	{
		char *setting;
		char *value;
	};
	hs_custom_command set_temp(
		"set_temp",
		"Sets temporally setting",
		HS_FUNC(
			auto info = static_cast<set_temp_args*>(args);
			return conf.setTempSetting(info->setting, info->value);
		),
		{ hs_type::string , hs_type::string },
		hs_type::boolean,
		"<string:setting> <string:value>"
	);
	// already a nop in both game and sapien so safe to reuse.
	g_halo_script_interface->RegisterCustomCommand(hs_opcode::test_network_storage_simulate, set_temp);
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

unsigned int calc_crc32_checksum(unsigned int *output, const BYTE *data, int size)
{
	typedef unsigned int __cdecl calc_crc32_checksum(unsigned int *output, const BYTE *data, int size);
	auto calc_crc32_checksum_impl = reinterpret_cast<calc_crc32_checksum*>(SwitchAddessByMode(0x00535E40, 0x504130, 0x4A84D0));
	return calc_crc32_checksum_impl(output, data, size);
}

bool crc32_unit_test()
{
	const static unsigned char test_data[] = "This is some test data";
	unsigned int halo_crc = 0xFFFFFFFFu;
	crc32::result h2codez_crc;

	calc_crc32_checksum(&halo_crc, test_data, sizeof(test_data));
	crc32::calculate(h2codez_crc, test_data, sizeof(test_data));
	bool is_good = LOG_CHECK(h2codez_crc == halo_crc);
	return LOG_CHECK(crc32::calculate(test_data, sizeof(test_data)) == crc32::calculate(&test_data)) && is_good;
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

	init_haloscript_patches();

	DetourTransactionCommit();

	if (is_debug_build())
	{
		if (!crc32_unit_test())
			pLog.WriteLog("crc unit tests failed");
	}
}
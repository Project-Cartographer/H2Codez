#include "H2EKCommon.h"
#include "stdafx.h"
#include "util/Patches.h"
#include "Psapi.h"
#include "DiscordInterface.h"
#include "TagInterface.h"
#include "util/Debug.h"
#include "util/array.h"
#include "HaloScript.h"
#include "HaloScript/hs_global_descriptions.h"
#include <cwchar>
#include <cassert>
#include <Shellapi.h>
#include <Shlwapi.h>
#include <CommDlg.h>
#include "util/crc32.h"
#include "util/process.h"

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

/*
	Override strings returned from h2alang to fix broken strings
*/
int WINAPI LoadStringW_Hook(HINSTANCE hInstance, UINT uID, LPWSTR lpBuffer, int cchBufferMax)
{ 
	if (GetModuleHandleW(L"H2alang") != hInstance)
		return LoadStringW_Orginal(hInstance, uID, lpBuffer, cchBufferMax);

	auto set_string = [&](const wchar_t *string) -> int
	{
		wcscpy_s(lpBuffer, cchBufferMax, string);
		return std::wcslen(lpBuffer);
	};

	if (310 <= uID && uID <= 318)
		return set_string(map_types[uID / 2 - 155]);

	switch (uID)
	{
		case 26: // org: open as text
			return set_string(L"Export as text");
		case 0x1018:
			return set_string(L"%s.%hs saved");
		case 0x12AE:
			return set_string(L"Unit Playtest");
		case 0xF3Cu:
			return set_string(L"%.0d--- importing %s.%hs ---\n"); // used to be "%*s--- importing %s.%s ---" but the printf was broke (it pushed too few args)
		case 0xF3Bu:
			return set_string(L"%.0d %s pitch range '%hs\n'");
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

std::string get_hs_command_description(const hs_command *cmd)
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

	// mark no-ops in toolkit
	const static std::vector<DWORD> sapien_nops = { 0x57B700, 0x57DDF0, 0x57E560 };
	const std::vector<DWORD> &nops = SwitchByMode({}, sapien_nops, {});
	for (auto addr : nops)
	{
		if (addr == reinterpret_cast<DWORD>(cmd->command_impl))
			usage += " [Game-only]";
	}

	return usage;
}

std::string get_hs_global_description(WORD id)
{
	static constexpr WORD globals_in_game[] = {
		17,  314, 315,
		321, 322, 331,
		338, 341, 342,
		343, 344, 345,
		346, 347, 348,
		349, 350, 351,
		352, 353, 354,
		355, 356, 357,
		442, 550, 628,
		629, 795, 796,
		803, 804, 805,
		806, 807, 808,
		809, 810, 811,
		812, 813, 814,
		815, 816, 817,
		818, 819, 983,
		1010,    1038
	};

	auto *current_var = g_halo_script_interface->global_table[id];
	std::string desc = "(";
	desc += current_var->name;
	desc += " <" + hs_type_string[current_var->type] + ">";
	desc += ")";

	bool is_usable_in_game = array_util::contains(globals_in_game, id);
	bool is_usable_in_toolkit = current_var->variable_ptr != nullptr;

	if (!is_usable_in_game && !is_usable_in_toolkit)
		desc += " [non-functional]";
	if (is_usable_in_game)
		desc += " [game]";
	if (hs_descriptions.count(static_cast<hs_global_id>(id))) {
		desc += "\n\t";
		desc += hs_descriptions[static_cast<hs_global_id>(id)];
	}
	return desc;
}

std::string H2CommonPatches::get_temp_name(const std::string &name_suffix)
{
	std::string name = std::tmpnam(nullptr);
	if (name_suffix.size() > 0)
		name += "." + name_suffix;
	return name;
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
		for (const hs_command *cmd : g_halo_script_interface->command_table)
		{
			fprintf(FilePtr, "%s\r\n", get_hs_command_description(cmd).c_str());
			fprintf(FilePtr, "%s\r\n\r\n", cmd->desc);
		}

		fprintf(FilePtr, "== Script Globals ==\r\n\r\n");
		for (WORD i = 0; i < g_halo_script_interface->get_global_table_count(); i++)
		{
			// used for padding, don't actual do anything
			if (g_halo_script_interface->global_table[i]->type == hs_type::nothing)
				continue;
			fprintf(FilePtr, "%s\r\n", get_hs_global_description(i).c_str());
		}

		fclose(FilePtr);
	}
	ShellExecuteA(NULL, NULL, file_name.c_str(), NULL, NULL, SW_SHOW);
}

void H2CommonPatches::dump_loaded_tags(const std::wstring folder)
{
	tags::s_tag_ilterator ilterator;
	for (datum tag = ilterator.next(); tag != datum::null(); tag = ilterator.next())
	{
		file_reference tag_data;
		if (LOG_CHECK(tags::get_tag_filo(&tag_data, tag)))
		{
			std::wstring tag_path = wstring_to_string.from_bytes(tags::get_name(tag));
			std::wstring tag_name_path = tag_path + std::wstring(L".")
				+ wstring_to_string.from_bytes(tags::get_group_definition(tag)->name);

			std::wstring old_path =  process::GetExeDirectoryWide() + std::wstring(L"\\")
				+  wstring_to_string.from_bytes(tag_data.path);

			HANDLE file_handle = CreateFileW(old_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			FILETIME modified_time;
			GetFileTime(file_handle, NULL, NULL, &modified_time);
			CloseHandle(file_handle);

			constexpr FILETIME last_bungie_time { 0xd8c82b00, 0x1c78bca }; // h2sapien.exe modification time
			if (CompareFileTime(&modified_time, &last_bungie_time) == -1)
				continue;

			std::wstring new_path = folder + std::wstring(L"\\") + tag_name_path;
			std::wstring new_dir = folder + L"\\";
			new_dir += tag_path.substr(0, tag_path.find_last_of(L"/\\"));

			int error_code = SHCreateDirectoryExW(NULL, new_dir.c_str(), NULL);
			if (!LOG_CHECK(error_code == ERROR_SUCCESS || error_code == ERROR_ALREADY_EXISTS))
				LOG_FUNC("dir_error: %d", error_code);
			CopyFileW(old_path.c_str(), new_path.c_str(), true);
		}
	}
}

std::string H2CommonPatches::get_h2ek_documents_dir()
{
	char h2_docs_folder[0x200];
	SHGetFolderPathA(0, CSIDL_PERSONAL, 0, 0, h2_docs_folder);// get user documents folder
	PathAppendA(h2_docs_folder, "Halo 2");
	return h2_docs_folder;
}

#define SUCCEEDED_LOG(expr) LOG_CHECK(SUCCEEDED(expr))

void H2CommonPatches::dump_loaded_tags()
{
	IFileDialog *pfd;
	if (SUCCEEDED_LOG(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
	{
		DWORD dwOptions;
		if (SUCCEEDED_LOG(pfd->GetOptions(&dwOptions)))
		{
			pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);
		}
		if (SUCCEEDED_LOG(pfd->Show(NULL)))
		{
			IShellItem *psi;
			if (SUCCEEDED_LOG(pfd->GetResult(&psi)))
			{
				wchar_t *path = nullptr;
				if (!SUCCEEDED_LOG(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &path)))
				{
					LOG_FUNC("Failed to get path");
				} else {
					dump_loaded_tags(path);
					CoTaskMemFree(path);
				}
				psi->Release();
			}
		}
		pfd->Release();
	}
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
	PatchCall(SwitchAddessByMode(0x006708E6, 0x005061A5, 0x005AEFF6), get_wide_halo_2_documents_path); // WRL export
	PatchCall(SwitchAddessByMode(0x00670B05, 0x005061C2, 0x005AF215), get_wide_halo_2_documents_path); // comments export
	if (game.process_type == H2Guerilla) {
		PatchCall(0x00430E76, get_wide_halo_2_documents_path); // working without patches
		PatchCall(0x00445FDA, get_wide_halo_2_documents_path); // not sure if working or broken, leaving as is for now
		PatchCall(0x00446928, get_wide_halo_2_documents_path); // breaks with patches
	}
}

typedef BOOL (WINAPI *T_FuncOpenFileNameW)(LPOPENFILENAMEW info);
T_FuncOpenFileNameW GetOpenFileNameWOriginal;
BOOL WINAPI GetOpenFileNameWHook(LPOPENFILENAMEW info)
{
	// check if it's the basic file select dialog
	if (reinterpret_cast<DWORD>(info->lpfnHook) == SwitchAddessByMode(0, 0x0040c450, 0x0069a0f7))
		info->Flags &= ~OFN_ENABLEHOOK; //  disable hook, and use default windows style
	return GetOpenFileNameWOriginal(info);
}

T_FuncOpenFileNameW GetSaveFileNameWOriginal;
BOOL WINAPI GetSaveFileNameWHook(LPOPENFILENAMEW info)
{
	// check if it's the basic file select dialog
	if (reinterpret_cast<DWORD>(info->lpfnHook) == SwitchAddessByMode(0, 0x0040c450, 0x0069a0f7))
		info->Flags &= ~OFN_ENABLEHOOK; //  disable hook, and use default windows style
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

LPTOP_LEVEL_EXCEPTION_FILTER
WINAPI
SetUnhandledExceptionFilter_hook(
	_In_opt_ LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter
)
{
	auto old_handler = Debug::get_expection_filter();
	Debug::set_expection_filter(lpTopLevelExceptionFilter);
	return old_handler;
}

typedef char __cdecl tags__fix_corrupt_fields(tag_block_defintions *def, tag_block_ref *data, int should_log);
tags__fix_corrupt_fields *tags__fix_corrupt_fields_org;
char __cdecl tags__fix_corrupt_fields___hook(tag_block_defintions *def, tag_block_ref *data, int should_log)
{
	return tags__fix_corrupt_fields_org(def, data, 1);
}

static void set_tag_data_max_size(size_t limit)
{
	// error message, check, set to
	std::array<size_t, 3> offsets_tool     = { 0x5DDD72, 0x5DDEAB, 0x5DDEB6 };
	std::array<size_t, 3> offsets_sapien   = { 0x551B02, 0x551C3B, 0x551C46 };
	std::array<size_t, 3> offsets_guerilla = { 0x5180E2, 0x51821B, 0x518226 };

	auto offsets = SwitchByMode(offsets_tool, offsets_sapien, offsets_guerilla);
	for (auto offset : offsets)
		WriteValue(offset + 1, limit);
}

void H2CommonPatches::Init()
{
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

	if (game.process_type != H2Tool) {
		tags__fix_corrupt_fields_org = reinterpret_cast<tags__fix_corrupt_fields*>(SwitchAddessByMode(0x52FEC0, 0x4B18E0, 0x485590));
		DetourAttach(&(PVOID&)tags__fix_corrupt_fields_org, tags__fix_corrupt_fields___hook);
	}

	fix_documents_path_string_type();

	haloscript_init();

	set_tag_data_max_size(0x3000000);

	// hook exception setter
	auto SetUnhandledExceptionFilterOrg = SetUnhandledExceptionFilter;
	DetourAttach(&(PVOID&)SetUnhandledExceptionFilterOrg, SetUnhandledExceptionFilter_hook);

	DetourTransactionCommit();

	// misc dev stuff
	if (is_debug_build())
	{
		if (!crc32_unit_test())
			pLog.WriteLog("CRC unit tests failed");

		FILE *FilePtr;
		std::string file_name = get_temp_name("hs_dump.txt");

		if (!fopen_s(&FilePtr, file_name.c_str(), "w"))
		{
			for (size_t i = 0; i < g_halo_script_interface->get_global_table_count(); i++)
			{
				const hs_global_variable *current_var = g_halo_script_interface->global_table[i];
				fprintf(FilePtr, "[%d : %d] %s\r\n", i, current_var->type, current_var->name);
			}
			fclose(FilePtr);
		}
	}
}

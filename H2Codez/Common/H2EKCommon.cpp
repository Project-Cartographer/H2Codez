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
#include "util/crc32.h"
#include "util/process.h"
#include "util/ScopedCOM.h"
#include <cwchar>
#include <cassert>
#include <Shellapi.h>
#include <Shlwapi.h>
#include <CommDlg.h>
#include <d3d9.h>
//#include <D3DX9Shader.h>
#include <d3dcompiler.h>

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
			return set_string(L"%*s pitch range '%hs'\n");
		case 0xF41u:
			return set_string(L"%*s permutation '%hs'\n");
		case 0x1457u:
			return set_string(L"%s is larger than 20 MiB, which exceeds engine limitations. Import may fail without warning.");
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
			std::wstring tag_path = utf8_to_utf16(tags::get_name(tag));
			std::wstring tag_name_path = tag_path + std::wstring(L".")
				+ utf8_to_utf16(tags::get_group_definition(tag)->name);

			std::wstring old_path =  process::GetExeDirectoryWide() + std::wstring(L"\\")
				+ utf8_to_utf16(tag_data.path);

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

typedef char __cdecl get_install_path_from_registry(wchar_t *path_1, int *max_len);
get_install_path_from_registry *get_install_path_from_registry_original;

char static __cdecl get_install_path_from_registry_hook(wchar_t *path, int *max_len)
{
	const static bool force_portable = conf.getBoolean("force_portable", false);

	if (!force_portable && get_install_path_from_registry_original(path, max_len))
		return true;
	auto install_directory_portable = process::GetExeDirectoryWide();
	wcscpy_s(path, *max_len, install_directory_portable.c_str());
	return true;
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

static bool __cdecl check_bitmap_dimension(int format, int type, __int16 dimension)
{
	return dimension > 0 && dimension <= max_bitmap_size;
}

Logs &getShaderLog()
{
	static Logs logger("shader.log", false);
	return logger;
}

static void dump_shader_code(const void* data, size_t size, const std::string& comment) {
	ScopedCOM<ID3DBlob> assembly;
	if (LOG_CHECK(D3DDisassemble(data, size, 0, comment.c_str(), &assembly) == S_OK && assembly.is_valid())) {
		getShaderLog().WriteLog("********************* \n%s\n", assembly->GetBufferPointer());
	}
}

static void compile_vertex_shader(const byte_ref& shader_code, byte_ref& compiled_shader, datum tag) {
	const std::string tag_name = tags::get_name(tag);
	if (is_debug_build() && compiled_shader.size > 0) {
		dump_shader_code(compiled_shader.address, compiled_shader.size, "old bytecode for " + tag_name);
	}
	ScopedCOM<ID3DBlob> code;
	ScopedCOM<ID3DBlob> error;
	static constexpr D3D_SHADER_MACRO macros { };
	auto result = D3DCompile(shader_code.address, shader_code.size, tag_name.c_str(), &macros, NULL, "main", "vs_2_0", D3DCOMPILE_DEBUG, 0, &code, &error);
	if (result == S_OK && code.is_valid()) {
		auto size = code->GetBufferSize();
		auto data = ASSERT_CHECK(code->GetBufferPointer());
		// dump assembly to output
		if (is_debug_build()) {
			dump_shader_code(data, size, "new bytecode for " + tag_name);
		}
		if (LOG_CHECK(compiled_shader.resize(size)))
			memcpy(compiled_shader.address, data, size);
	} else if (error.is_valid()) {
		auto error_msg = static_cast<char*>(error->GetBufferPointer());
		LOG_FUNC("error = %s", error_msg);
	} else {
		LOG_FUNC("Failed to compile shader, but got no error? result=%d", result);
	}
}

struct vertex_shader_classification_block {
	IDirect3DVertexShader9 *shader;
	byte_ref compiled_shader;
	byte_ref code;
};
static bool __cdecl vertex_shader_classification_block_postprocess_proc(datum owner_tag_index, vertex_shader_classification_block *element, bool for_editor) {
	if ((is_debug_build() || element->compiled_shader.size == 0) && element->code.size > 1)
		compile_vertex_shader(element->code, element->compiled_shader, owner_tag_index);
	if (!for_editor)
		element->code.resize(0);
	return true;
}

// fast versions of the memory allocation functions used by the toolkit
namespace fast_memory_alloc {

	static constexpr int memory_tag = 'sht!';

	struct memory_info {
		int tag;
		size_t alignment;
	};

	inline static bool should_debug() {
		return is_debug_build();
	}

	// translate alignment format
	size_t static inline debug_translate_aligment(unsigned char alignment_power) {
		return (alignment_power != 0) ? 1 << alignment_power : 0;
	}

	// get memory_info/allocation base
	static inline memory_info* debug_get_base(void* data) {
		return data ? reinterpret_cast<memory_info*>(reinterpret_cast<uintptr_t>(data) - sizeof(memory_info)) : nullptr;
	}

	// convert base to user pointer
	static inline void* debug_base_to_pointer(void* base) {
		if (!base)
			return nullptr;
		return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(base) + sizeof(memory_info));
	}

	// init pointer info
	static inline void* debug_init_pointer(void* data, size_t alignment) {
		if (!data)
			return nullptr;
		auto info = reinterpret_cast<memory_info*>(data);
		info->tag = memory_tag;
		info->alignment = alignment;
		return debug_base_to_pointer(data);
	}

	// check if a pointer could have been returned by our allocator
	bool inline static debug_is_pointer_valid(void* data) {
		auto base = debug_get_base(data);
		if (!base)
			return true;
		return base->tag == memory_tag;
	}

	static inline void* debug_allocate(size_t size, unsigned char alignment_power) {
		auto align = debug_translate_aligment(alignment_power);
		void* base;
		if (align)
			base = _aligned_offset_malloc(size + sizeof(memory_info), align, sizeof(memory_info));
		else
			base = malloc(size + sizeof(memory_info));
		return debug_init_pointer(base, align);
	}

	static inline void debug_free(void* pointer) {
		if (should_debug())
			ASSERT_CHECK(debug_is_pointer_valid(pointer));
		auto base = debug_get_base(pointer);
		if (!base)
			return;
		base->tag = 0;
		if (base->alignment)
			_aligned_free(base);
		else
			free(base);
	}

	static inline void* debug_realloc(void* pointer, size_t size, unsigned char alignment_power) {
		if (should_debug())
			ASSERT_CHECK(debug_is_pointer_valid(pointer));
		if (!pointer && size)
			return debug_allocate(size, alignment_power);
		if (!size) {
			if (pointer)
				debug_free(pointer);
			return nullptr;
		}
		auto align = debug_translate_aligment(alignment_power);
		auto base = debug_get_base(pointer);
		if (should_debug())
			ASSERT_CHECK(base->alignment == align);
		void* new_base;
		if (align)
			new_base = _aligned_offset_realloc(base, size + sizeof(memory_info), align, sizeof(memory_info));
		else
			new_base = realloc(base, size + sizeof(memory_info));
		return debug_base_to_pointer(new_base);
	}
}

static void* __cdecl debug_allocate_fast_hook(size_t size, unsigned char alignment_power, const char *file, int line, const char *type, const char *subtype, const char *name) {
	if (LOG_CHECK(size))
		return fast_memory_alloc::debug_allocate(size, alignment_power);
	else
		return nullptr;
}

static void __cdecl debug_free_fast_hook(void *pointer, const char *file, int line) {
	fast_memory_alloc::debug_free(pointer);
}

static void* __cdecl debug_reallocate_fast_hook(void *pointer, size_t size, unsigned char alignment_power, const char *file, int line, const char *type, const char *subtype, const char *name) {
	return fast_memory_alloc::debug_realloc(pointer, size, alignment_power);
}

static bool __cdecl debug_is_valid_allocation_fast_hook(void *pointer) {
	if (!pointer)
		return false;
	return fast_memory_alloc::debug_is_pointer_valid(pointer);
}

void __cdecl benchmark_mem() {
	auto start = clock();
	for (int j = 0; j < 0x500; j++) {
		void* pointers[0x2000];
		for (int i = 0; i < ARRAYSIZE(pointers); i++)
			pointers[i] = HEK_DEBUG_MALLOC(0x300, numerical::is_power_of_two(i) ? 1 : 0);
		for (int i = 0; i < ARRAYSIZE(pointers); i++)
			HEK_DEBUG_FREE(pointers[i]);
	}
	wprintf(L"took %d ms\n", (clock() - start) / (CLOCKS_PER_SEC / 10));
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

	if (game.process_type != H2Guerilla) {
		auto offset = SwitchAddessByMode(0x00A24E68, 0x00A5F8F8, 0);
		WritePointer(offset, vertex_shader_classification_block_postprocess_proc);
	}

	get_install_path_from_registry_original = reinterpret_cast<get_install_path_from_registry*>(SwitchByMode(0x589D30, 0x4BA7E0, 0x48A070));
	DetourAttach(&(PVOID&) get_install_path_from_registry_original, get_install_path_from_registry_hook);

	WriteJmp(SwitchByMode(0x720FF0, 0x6FFCE0, 0x65D030), check_bitmap_dimension);

	fix_documents_path_string_type();

	haloscript_init();

	set_tag_data_max_size(0x5000000);

	// hook exception setter
	auto SetUnhandledExceptionFilterOrg = SetUnhandledExceptionFilter;
	DetourAttach(&(PVOID&)SetUnhandledExceptionFilterOrg, SetUnhandledExceptionFilter_hook);

	DetourTransactionCommit();
	if (game.process_type != H2Guerilla && conf.getBoolean("disable_debug_memory_allocator", false)) {
		WriteJmp(debug_memory::get_offsets().allocate, debug_allocate_fast_hook);
		WriteJmp(debug_memory::get_offsets().reallocate, debug_reallocate_fast_hook);
		WriteJmp(debug_memory::get_offsets().free, debug_free_fast_hook);
		if (game.process_type == H2Tool)
			WriteJmp(0x52A870, debug_is_valid_allocation_fast_hook);
	}
	//PatchCall(0x5270D5, benchmark_mem);
	// misc dev stuff
	if (is_debug_build())
	{
		if (!crc32_unit_test())
			getLogger().WriteLog("CRC unit tests failed");

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

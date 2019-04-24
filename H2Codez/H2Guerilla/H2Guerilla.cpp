#include "stdafx.h"
#include "H2Guerilla.h"
#include "util\Patches.h"
#include "util\process.h"
#include "Resources\resource.h"
#include "Common\H2EKCommon.h"
#include "Common\FiloInterface.h"
#include "Common\BlamBaseTypes.h"
#include "Common\TagDefinitions.h"
#include "Common\AssemblyLayoutGenerator.h"
#include "Tags\ScenarioTag.h"
#include "template_defintions.h"

typedef int(__fastcall *toggle_expert_mode)(int thisptr, int __unused);
toggle_expert_mode toggle_expert_mode_orginal;

typedef HMENU (WINAPI *LoadMenuTypedef)(_In_opt_ HINSTANCE hInstance, _In_ LPCWSTR lpMenuName);
static LoadMenuTypedef LoadMenuOrginal;

typedef void(__fastcall *CCmdUI__Enable)(void *thisptr, BYTE _, int a2);
CCmdUI__Enable CCmdUI__Enable_Orginal;

typedef int (__fastcall *CCmdTarget__OnCmdMsg)(void *thisptr, BYTE _, unsigned int msg, void *a3, void *a4, void *AFX_CMDHANDLERINFO);
CCmdTarget__OnCmdMsg CCmdTarget__OnCmdMsg_Orginal;

typedef char *(__cdecl *hs_remote_generate_command)(char *command_name, void **args, signed int arg_count, char *output, rsize_t output_size);
hs_remote_generate_command hs_remote_generate_command_orginal;

#define INVALID_HMENU_VALUE (HMENU)INVALID_HANDLE_VALUE

static std::unordered_map<DWORD, HMENU> menu_map
{
	{ 14,   INVALID_HMENU_VALUE },
	{ 6014, INVALID_HMENU_VALUE },
	{ 1000, INVALID_HMENU_VALUE },
};

bool show_hidden_fields = true;

inline static void CheckItem(UINT item, bool enable)
{
	for (const auto menu : menu_map)
		CheckMenuItem(menu.second, item, MF_BYCOMMAND | (enable ? MF_CHECKED : MF_UNCHECKED));
}

inline static void EnableItem(UINT item, bool enable)
{
	for (const auto menu : menu_map)
		EnableMenuItem(menu.second, item, enable ? MF_ENABLED : MF_DISABLED);
}

void update_ui()
{
	CheckItem(ID_EDIT_ADVANCEDSHADERVIEW, conf.getBoolean("disable_templete_view"));
}

/* Capture menu input */
int __fastcall CCmdTarget__OnCmdMsg_hook(void *thisptr, BYTE _, unsigned int msg, void *a3, void *a4, void *AFX_CMDHANDLERINFO)
{
	auto toggle_boolean = [](std::string setting, bool default_value = false)
	{
		conf.setBoolean(setting, !conf.getBoolean(setting, default_value));
	};

	if (!AFX_CMDHANDLERINFO && !a3 && !a4) {
		switch (msg) {
		case ID_EDIT_ADVANCEDSHADERVIEW:
			toggle_boolean("disable_templete_view", false);
			update_ui();
			return true;

		case ID_FILE_NEWINSTANCE:
			process::newInstance();
			return true;

		case SCRIPT_DOC:
			H2CommonPatches::generate_script_doc();
			return true;

		case SHOW_HIDDEN_FIELDS:
			show_hidden_fields = !show_hidden_fields;
			H2GuerrilaPatches::update_field_display();
			conf.setBoolean("show_hidden_fields", show_hidden_fields);
			return true;
		case DUMP_XML_DEFINITION:
			TagDefinitions::dump_as_xml();
			AssemblyLayoutGenerator::DumpAllTags();
			return true;
		}
	}
	return CCmdTarget__OnCmdMsg_Orginal(thisptr, 0, msg, a3, a4, AFX_CMDHANDLERINFO);
}

/* Enable custom items */
void __fastcall CCmdUI__Enable_Hook(void *thisptr, BYTE _, int a2)
{
	CCmdUI__Enable_Orginal(thisptr, 0, a2);
	EnableItem(ID_EDIT_ADVANCEDSHADERVIEW, true);
	EnableItem(ID_FILE_NEWINSTANCE, true);
	EnableItem(SCRIPT_DOC, true);
	EnableItem(SHOW_HIDDEN_FIELDS, true);
	EnableItem(DUMP_XML_DEFINITION, true);
}

/* Override menu loaded by guerilla */
static HMENU WINAPI LoadMenuHook(_In_opt_ HINSTANCE hInstance, _In_ LPCWSTR lpMenuName)
{
	DWORD MenuId = reinterpret_cast<DWORD>(lpMenuName);
	if (hInstance == GetModuleHandle(NULL)) {
		pLog.WriteLog("LoadMenuHook: %d", lpMenuName);
		if (LOG_CHECK(menu_map.find(MenuId) != menu_map.end()))
			return menu_map[MenuId];
	}
	return LoadMenuOrginal(hInstance, lpMenuName);
}

int __fastcall toggle_expert_mode_hook(int thisptr, int __unused)
{
	int return_value = toggle_expert_mode_orginal(thisptr, 0);
	char *expert_mode = CAST_PTR(char*, 0x9AF809);
	conf.setBoolean("expert_mode", *expert_mode);
	return return_value;
}

bool __cdecl create_temp_filo(filo *data, LPCSTR location)
{
	FiloInterface::init_filo(data, H2CommonPatches::get_temp_name(), false);
	return true;
}

char *__cdecl hs_remote_generate_command_hook(char *command_name, void **args, signed int arg_count, char *output, rsize_t output_size)
{
	hs_remote_generate_command_orginal(command_name, args, arg_count, output, output_size);
	// crashes because the 'scnr' tag isn't loaded.
	//HaloScriptCommon::hs_runtime_execute(output);
	MessageBoxA(NULL, output, "HS Remote Command", 0);
	return output;
}

void __fastcall guerilla_wide_string__append__hook(int this_ptr, int _, int this_ptr_copy, void *a4, char *string_start, char *string_end, int a7)
{
	typedef void(__fastcall *t_guerilla_wide_string__append)(int this_ptr, int, int this_ptr_copy, void *a4, const wchar_t *string_start, const wchar_t *string_end, int a7);
	auto guerilla_wide_string__append = reinterpret_cast<t_guerilla_wide_string__append>(0x475BC0);

	std::wstring wide_string;
	size_t string_max_len = (string_end - string_start) + 1;

	wide_string = wstring_to_string.from_bytes(string_start, string_start + strnlen_s(string_start, string_max_len));

	const wchar_t *new_string_start = wide_string.c_str();
	const wchar_t *new_string_end = new_string_start + wide_string.size() - 1;
	guerilla_wide_string__append(this_ptr, 0, this_ptr_copy, a4, new_string_start, new_string_end, a7);

	// Don't ask how this fixes anything, I don't know
	Sleep(500);
}

int WINAPI GetMenuItemCountHook(
	HMENU hMenu
)
{
	// hide the version element from Guerilla.
	DWORD hMenuId = reinterpret_cast<DWORD>(hMenu);
	if (menu_map.find(hMenuId) != menu_map.end())
		return 5;
	else
		return GetMenuItemCount(hMenu);
}

const static tempate_constructor c_shader_template_con = reinterpret_cast<tempate_constructor>(0x637220);
const static tempate_constructor c_sound_effect_template_con = reinterpret_cast<tempate_constructor>(0x51A7D0);
const static tempate_constructor c_particle_movement_template_con = reinterpret_cast<tempate_constructor>(0x51A040);
const static tag_block_template_def tag_templates[]
{
	{'shad', 1, c_shader_template_con },
	{'prt3', 0, c_shader_template_con },
	// {'ssfx', 0, c_sound_effect_template_con }, // causes crashes
	{'pmov', 1, c_particle_movement_template_con },
};

const tag_block_template_def *__cdecl get_tag_block_template(blam_tag block)
{
	if (conf.getBoolean("disable_templete_view", false))
		return nullptr;
	for (auto i = 0; i < ARRAYSIZE(tag_templates); i++)
	{
		auto tag_template = &tag_templates[i];
		if (tag_template->type == block)
			return tag_template;
	}
	return nullptr;
}

void H2GuerrilaPatches::update_field_display()
{
	CheckItem(SHOW_HIDDEN_FIELDS, show_hidden_fields);
	if (show_hidden_fields) {
		// Display All Fields
		NopFill(0x44CDAA, 0x8);
	}
	else {
		BYTE orginal_code[8] = { 0x84, 0xC0, // test al, al
			0x0F, 0x84, 0x72, 0x01, 0x00, 0x00 }; // jz loc_44CF24
		WriteBytes(0x44CDAA, orginal_code, 8);
	}
}

void H2GuerrilaPatches::Init()
{
	for (auto &menu : menu_map)
		menu.second = LoadMenuA(g_hModule, MAKEINTRESOURCEA(GUERILLA_MENU));
#pragma region Patches
	// Start patches copied from OpenSauce

	// Allow Base Tag Group Creation
	NopFill(0x409FE0, 11);

	// Display all Tags
	NopFill(0x409FEE, 0x23);
	NopFill(0x4476E7, 0x23);

	BYTE patch_can_display_tag[0x3] = { 0xB0, 0x01, 0xC3 }; // mov al, 1; retn
	WriteBytes(0x47FB10, patch_can_display_tag, 0x3);

	BYTE patch_file_extenstion_check[0x4] = { 0xE9, 0xCD, 0x00, 0x00 }; // mov al, 1; retn
	WriteBytes(0x47FB8D, patch_file_extenstion_check, 0x4);

	// End patches copied from OpenSauce

	// Doesn't fix the underlying issue, just removes the error message allowing one more scenario to be opened
	BYTE patch_out_of_resources[0x2] = { 0xEB, 0x03 };
	WriteBytes(0x44CF1F, patch_out_of_resources, 0x2);
	NopFill(0x44CDD0, 0x5);

	// A messy fix for script text getting cut off
	WriteValue(0x402FCE + 1, 0x40000);
	WriteValue(0x402F0D + 1, 0x40000);

	// allow other processes to read files open with fopen_s
	WriteValue(0x006B1614 + 1, _SH_DENYWR);

	// patch code for embedded tool console to use the right exe name
	WritePointer(0x004761AC + 1, "H2tool ");

	PatchCall(0x00476408, guerilla_wide_string__append__hook);
	PatchCall(0x00476494, guerilla_wide_string__append__hook);

	// re-add removed information about scenario types.
	constexpr static tag_enum_map_element scenario_types[5] =
	{
		tag_enum_map_element("Singleplayer", scnr_tag::Type::Singleplayer),
		tag_enum_map_element("Multiplayer",  scnr_tag::Type::Multiplayer),
		tag_enum_map_element("Mainmenu",     scnr_tag::Type::Mainmenu),
		tag_enum_map_element("Multiplayer Shared",   scnr_tag::Type::MultiplayerShared),
		tag_enum_map_element("Singleplayer Shared",  scnr_tag::Type::SingleplayerShared),
	};
	WriteValue(0x00901920, ARRAYSIZE(scenario_types));
	WritePointer(0x00901924, scenario_types);

	// fix "help" menu getting replaced by "window" menu
	PatchAbsCall(0x00686D0A, GetMenuItemCountHook);

	// allow manipulating tag templates
	WriteJmp(0x48E730, get_tag_block_template);

	constexpr static blam_tag widget_tags[] =
	{
		'ant!',
		'devo',
		'whip',
		'BooM',
		'tdtl',
		'clwd',
		blam_tag::null()
	};
	static_assert(sizeof(widget_tags) == ARRAYSIZE(widget_tags) * 4, "array element size check failed!");

	WritePointer(0x98412C, widget_tags);

#pragma endregion

#pragma region Hooks
	
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	toggle_expert_mode_orginal = CAST_PTR(toggle_expert_mode,0x4024A0);
	DetourAttach(&(PVOID&)toggle_expert_mode_orginal, toggle_expert_mode_hook);

	LoadMenuOrginal = LoadMenuW;
	DetourAttach(&(PVOID&)LoadMenuOrginal, LoadMenuHook);

	CCmdUI__Enable_Orginal = CAST_PTR(CCmdUI__Enable, 0x67F09A);
	DetourAttach(&(PVOID&)CCmdUI__Enable_Orginal, CCmdUI__Enable_Hook);

	// hook menu input
	CCmdTarget__OnCmdMsg_Orginal = CAST_PTR(CCmdTarget__OnCmdMsg, 0x67EE0F);
	DetourAttach(&(PVOID&)CCmdTarget__OnCmdMsg_Orginal, CCmdTarget__OnCmdMsg_hook);

	// generates scripts for use by the remote command system
	hs_remote_generate_command_orginal = reinterpret_cast<hs_remote_generate_command>(0x0051DC30);
	DetourAttach(&(PVOID&)hs_remote_generate_command_orginal, hs_remote_generate_command_hook);

	// move temp files into the standard temp folder
	void *hook_temp_filo = reinterpret_cast<void*>(0x48C6F0);
	DetourAttach(&hook_temp_filo, create_temp_filo);
	DetourTransactionCommit();
#pragma endregion

	WriteValue(0x9AF809, (BYTE)conf.getBoolean("expert_mode", 1)); // set is_expert_mode to one
	show_hidden_fields = conf.getBoolean("show_hidden_fields", true);

	update_field_display();
	update_ui();
}

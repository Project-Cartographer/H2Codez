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

typedef int (__fastcall *CCmdTarget__OnCmdMsg)(void *thisptr, BYTE _, unsigned int nID, int nCode, void *pExtra, void *AFX_CMDHANDLERINFO);
CCmdTarget__OnCmdMsg CCmdTarget__OnCmdMsg_Orginal;

typedef char *(__cdecl *hs_remote_generate_command)(char *command_name, void **args, signed int arg_count, char *output, rsize_t output_size);
hs_remote_generate_command hs_remote_generate_command_orginal;

#define INVALID_HMENU_VALUE (HMENU)INVALID_HANDLE_VALUE

static std::unordered_map<DWORD, HMENU> menu_map
{
	{ 11,   INVALID_HMENU_VALUE },
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
	CheckItem(SHOW_ALL_HIDDEN_FIELDS, conf.getBoolean("show_all_fields"));
}

/* Capture menu input */
int __fastcall CCmdTarget__OnCmdMsg_hook(void *thisptr, BYTE _, unsigned int nID, int nCode, void *pExtra, void *AFX_CMDHANDLERINFO)
{
	auto toggle_boolean = [](std::string setting, bool default_value = false)
	{
		conf.setBoolean(setting, !conf.getBoolean(setting, default_value));
	};

	if (!AFX_CMDHANDLERINFO && !nCode && !pExtra) {
		switch (nID) {
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
		case SHOW_ALL_HIDDEN_FIELDS:
			toggle_boolean("show_all_fields", false);
			update_ui();
			return true;
		}
	}
	return CCmdTarget__OnCmdMsg_Orginal(thisptr, 0, nID, nCode, pExtra, AFX_CMDHANDLERINFO);
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
	EnableItem(SHOW_ALL_HIDDEN_FIELDS, show_hidden_fields);
}

/* Override menu loaded by guerilla */
static HMENU WINAPI LoadMenuHook(_In_opt_ HINSTANCE hInstance, _In_ LPCWSTR lpMenuName)
{
	DWORD MenuId = reinterpret_cast<DWORD>(lpMenuName);
	if (hInstance == GetModuleHandle(NULL)) {
		getLogger().WriteLog("LoadMenuHook: %d", lpMenuName);
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

bool __cdecl create_temp_filo(file_reference *data, LPCSTR location)
{
	file_reference temp_filo(H2CommonPatches::get_temp_name(), false);
	*data = temp_filo;
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

static inline bool is_tag_field_group_blacklisted(tag_field* tag_field)
{
	auto group = tag_field->group_tag;
	return group == 'fnom' || group == 'fnop' || group == 'fnin' || group == 'fnir';
}

static bool __fastcall field_information__should_hide(field_information *thisptr)
{
	ASSERT_CHECK(thisptr);
	ASSERT_CHECK(thisptr->field);
	
	/*
		Hide blocks that can't be used
	*/
	if (thisptr->field->type == tag_field::block)
	{
		auto block_def = reinterpret_cast<tag_block_defintions*>(thisptr->field->defintion);
		if (block_def->max_count == 0)
			return true;
	}

	if (show_hidden_fields && conf.getBoolean("show_all_fields", false))
		return false;

	/* Hide black listed groups */
	if (is_tag_field_group_blacklisted(thisptr->field))
		return true;

	std::string name = thisptr->field->name.get_string();
	/* 
		Hide unnamed fields needs to check if the string is not null due to a quirk of original code
	*/
	if (thisptr->field->name.id && name.length() == 0)
		return true;
	
	/* Finally hide based on name */
	bool should_hide = name.find('~') != std::string::npos;

	return should_hide && !show_hidden_fields;
}

static void fix_wgit_menu_id(tag_enum_def *enum_data)
{
	static constexpr editor_string new_enum_names[] = {
		"Restore Controller Defaults",
		"Video Settings",
		"Audio Settings",
		"Keyboard Settings Menu",
		"Pause Settings",
		"Keyboard Settings",
		"Video Settings Qtr",
		"Audio Settings Qtr",
		"Volume Settings",
		"Sound Quality",
		"EAX",
		"Audio Hardware 3D",
		"Speaker Config",
		"Restore Audio Defaults",
		"Resolution",
		"Aspect Ratio",
		"Display Mode",
		"Brightness Level",
		"Gamma Setting",
		"Anti-Aliasing",
		"Resize HUD",
		"Restore Video Defaults",
		"Search Option Maps",
		"Search Option Gametype",
		"Search Option Variant",
		"Search Option Gold Only",
		"Search Option Dedicated Servers",
		"Search Option Max Players",
		"Search Option Favorites",
		"Search Option Show Full Games",
		"Safe Area",
		"Find Game Menu",
		"Search Options",
		"UNUSED",
		"UNUSED",
		"LOD Setting",
		"Refresh",
		"ESRB Warning",
		"Resolution Confirmation",
		"Invert KB Look",
		"Restore Default Keyboard Settings",
		"Network Adapter Settings",
		"About Dialog"
	};
	static editor_string enum_names[0x12A];
	memcpy(enum_names, enum_data->names, enum_data->count * sizeof(char*));
	memcpy(&enum_names[0xFF], new_enum_names, sizeof(new_enum_names));
	enum_names[0xEF] = "Extra Settings";
	enum_names[0xF1] = "Extras Enabled Dialog";

	enum_data->names = enum_names;
	enum_data->count = ARRAYSIZE(enum_names);
}

static void fix_wgit_button_key_type(tag_enum_def* enum_data)
{
	static editor_string enum_names[0x1A];
	memcpy_s(enum_names, sizeof(enum_names), enum_data->names, enum_data->count * sizeof(char*));
	enum_names[0x17] = "X=DELETE A=EDIT B=DONE";
	enum_names[0x18] = "X=SORT Y=REFRESH A=SELECT B=BACK";
	enum_names[0x19] = "A=ACCEPT B=CANCEL";

	enum_data->names = enum_names;
	enum_data->count = ARRAYSIZE(enum_names);
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
	/* Works by patching the string conversion function */
	WriteValue(0x402FCE + 1, 0x40000);
	WriteValue(0x402F0D + 1, 0x40000);

	// allow other processes to read files open with fopen_s
	WriteValue(0x006B1614 + 1, _SH_DENYWR);

	// Add surface index to firing position definition.
	WriteValue<tag_field>(0x96ED68, { tag_field::long_integer, "surface index", NULL, blam_tag::null() });

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

	const tag_field inverse_matrix4x3_fieldset[] = {
		{ tag_field::real, "inverse scale*"             },
		{ tag_field::real_vector_3d, "inverse forward*" },
		{ tag_field::real_vector_3d, "inverse left*"    },
		{ tag_field::real_vector_3d, "inverse up*"      },
		{ tag_field::real_point_3d, "inverse position*" },
	};

	// Someone at bungie messed up this def, technically wrong in all the tools but it only matters in guerilla
	WriteArray(0x94FDD8, &inverse_matrix4x3_fieldset);

	// Replace guerilla logic for deciding if we should show a tag field
	WriteJmp(0x477230, field_information__should_hide);

	fix_wgit_menu_id(reinterpret_cast<tag_enum_def*>(0x93A454));
	fix_wgit_button_key_type(reinterpret_cast<tag_enum_def*>(0x93A77C));

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

	WriteValue<BYTE>(0x9AF809, conf.getBoolean("expert_mode", 1)); // set is_expert_mode to one
	show_hidden_fields = conf.getBoolean("show_hidden_fields", true);

	update_field_display();
	update_ui();
}

#include "..\stdafx.h"
#include "ToolCommandDefinitions.inl"
#include "H2Tool_extra_commands.inl"
#include "..\Common\H2EKCommon.h"
#include "../util/Patches.h"
#include "../Version.h"
#include "../Tags/ScenarioTag.h"
#include <regex>

using namespace HaloScriptCommon;

//I should better mention  the H2tool version i am using
//tool  debug pc 11081.07.04.30.0934.main  Apr 30 2007 09:37:08
//Credits to Kornmann https://bitbucket.org/KornnerStudios/opensauce-release/wiki/Home


static const s_tool_command* h2tool_extra_commands[] = {
	&import_model_render,
	&import_model_collision,
	&import_model_physics,
	&import_model,
	&import_model_animations,
	import_class_monitor_structures,
	import_class_monitor_bitmaps,
	&import_sound,
	&tool_build_structure_from_jms,
	&h2dev_extra_commands_defination,
	&list_extra_commands,

};

int __cdecl s_tool_command_compare(void *, const void* lhs, const void* rhs)
{
	const s_tool_command* _lhs = *CAST_PTR(const s_tool_command* const*, lhs);
	const s_tool_command* _rhs = *CAST_PTR(const s_tool_command* const*, rhs);

	const wchar_t* l = _lhs->name;
	const wchar_t* r = _rhs->name;
	if (l[0] == L'~') l++;
	if (r[0] == L'~') r++;

	return _wcsicmp(l, r);
}

void _H2ToolAttachHooks()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());


	//  DetourAttach(&(PVOID&)pPlayerSpeed,OnPlayerSpeed);	

	//  DetourAttach(&(PVOID&)pcreatechar,h_CreateCharacter);
	//DetourAttach(&(PVOID&)pXYZ, h_XYZUpdate);


	DetourTransactionCommit();
	return;
}
void _H2ToolDetachHooks()
{
	//DetourDetach(&(PVOID&)pgameoptions,h_GetGameOptions);
	// DetourDetach(&(PVOID&)pMapSelect,h_MPMapSelect);
	// DetourDetach(&(PVOID&)psquadsettings,h_SquadSettings);	
	return;
}
#pragma region structure_import Tweeks
void H2ToolPatches::Increase_structure_import_size_Check()
{
	//if ( FileSize.LowPart > 0x1400000 && !FileSize.HighPart )
	 ///1400000h =  20971520 BYTES ~ 20 MB

/*
.text:0041F836 C90 cmp     dword ptr [esp+0C90h+FileSize], 1400000h ; Compare Two Operands <-Change This Size
.text:0041F83E C90 jbe     loc_41F8D9                      ; Jump if Below or Equal (CF=1 | ZF=1)
.text:0041F83E
.text:0041F844 C90 cmp     dword ptr [esp+0C90h+FileSize+4], ebx ; Compare Two Operands
.text:0041F848 C90 jnz     loc_41F8D9 
*/


	//increasing the 20 MB File Import Check
	DWORD new_size = 0x1400000 * TOOL_INCREASE_FACTOR; ///671.08864 MB
	WriteValue(0x41F836 + 4, new_size);

	// push <max file size>
	// TODO: check this
	// WriteValue(0x589183 + 1, new_size);
}
void H2ToolPatches::structure_bsp_geometry_collision_check_increase()
{
//return collision_surfaces_count <= 0x7FFF && edges_count <= 0xFFFF && collision_vertices_count <= 0xFFFF;
	///0x7FFF = 32767
	///0xFFFF = 65535
/*
.text:005A2D50 000 cmp     [esp+collision_surfaces_count], 7FFFh ; Compare Two Operands
.text:005A2D58 000 jg      short loc_5A2D6E                ; Jump if Greater (ZF=0 & SF=OF)
.text:005A2D58
.text:005A2D5A 000 mov     eax, 0FFFFh
.text:005A2D5F 000 cmp     [esp+a2], eax 
*/
   //increasing the collision_surfaces_count
	DWORD collision_surfaces_count = 0x7FFF * TOOL_INCREASE_FACTOR; /// 0xFFFE0
	WriteValue(0x5A2D50 + 4, collision_surfaces_count);


	//increasing the edges_vertices_count
	DWORD edges_vertices_count = 0xFFFF * TOOL_INCREASE_FACTOR; /// 0x1FFFE0
	WriteValue(0x5A2D5A + 1, edges_vertices_count);

	///Also Patching in the error_proc method incase we ever hit this Limit :)
/*
.text:00464C50 118 push    0FFFFh
.text:00464C55 11C push    eax
.text:00464C56 120 push    0FFFFh
.text:00464C5B 124 push    ecx
.text:00464C5C 128 push    7FFFh
*/
	WriteValue(0x464C50 + 1, edges_vertices_count);
	WriteValue(0x464C56 + 1, edges_vertices_count);
	WriteValue(0x464C5C + 1, collision_surfaces_count);


}
void H2ToolPatches::structure_bsp_geometry_3D_check_increase()
{
	// return nodes_count < &unk_800000 && planes_count < 0x8000 && leaves_count < &unk_800000;
	/// planes_count =0x8000 = 32768 
/*
.text:005A2CFB 000 cmp     [esp+planes_count], 32768       ; Compare Two Operands
.text:005A2D03 000 jge     short loc_5A2D0E                ; Jump if Greater or Equal (SF=OF)
.text:005A2D03
*/
	////increasing the planes_count Check
	DWORD new_planes_count = 0x8000 *TOOL_INCREASE_FACTOR; /// 0x100000
	WriteValue(0x5A2CFB + 4, new_planes_count);

}
void H2ToolPatches::structure_bsp_geometry_2D_check_increase()
{
/*
.text:00464BA5 118 push    0        <- b_DONT_CHECK variable is set to false by default .Need to make it true
.text:00464BA7 11C push    ecx
.text:00464BA8 120 push    edx
.text:00464BA9 124 call    collision_bsp_2d_vertex_check   ; Call Procedure
*/
	WriteBytes(0x464BA5 + 1, new BYTE{1}, 1);

	//No Need to modify the Proc error here cuz it will never hit :)
}

static const int BSP_MAX_DEPTH = 512;

static const int function_epilog = 0x00465797;
static void __declspec(naked) bsp_depth_check()
{
	__asm {
		add eax, 1
		cmp eax, BSP_MAX_DEPTH
		jle continue_compile
		jmp abort_with_error

		continue_compile:
		// remove return address
		pop eax
	    // push function epilog for ret
		push function_epilog
		ret


		abort_with_error:
		ret
	}
}

void structure_bsp_depth_check_increase()
{
	// remove old check
	NopFill(0x00465726, 0xA);

	// write call to our check
	WriteCall(0x00465726, bsp_depth_check);
}

void H2ToolPatches::Increase_structure_bsp_geometry_check()
{
	H2PCTool.WriteLog("Increasing structure_bsp_geometry checks");
	structure_bsp_geometry_2D_check_increase();
	structure_bsp_geometry_3D_check_increase();
	structure_bsp_geometry_collision_check_increase();
	structure_bsp_depth_check_increase();
}

#pragma endregion

#pragma region Shared_tag_removal_scheme
//A reference to H2EK_OS tools SansSharing.inl
/*
* shared tag scheme removal changes

.text:005887DF                 push    1               ; CHANGE THIS TO FALSE
.text:005887E1                 mov     esi, eax
.text:005887E3                 call    _postprocess_animation_data

.text:00588C21                 push    1               ; CHANGE THIS TO FALSE
.text:00588C23                 push    edx
.text:00588C24                 mov     esi, eax
.text:00588C26                 call    _build_cache_file_prepare_sound_gestalt

.text:00588D52                 push    1               ; CHANGE THIS TO FALSE
.text:00588D54                 push    0
.text:00588D56                 push    0
.text:00588D58                 push    0
.text:00588D5A                 lea     edx, [esp+1F94h+multiplayer_shared_dependency_graph]
.text:00588D61                 mov     esi, eax
.text:00588D63                 push    edx
.text:00588D64                 lea     eax, [esp+1F98h+cache_header]
.text:00588D6B                 push    eax
.text:00588D6C                 lea     edx, [esp+1F9Ch+scenario_name_wide]
.text:00588D70                 call    _build_cache_file_add_tags ; ecx = s_shared_tag_index

.text:00588DCD                 push    1               ; CHANGE THIS TO FALSE
.text:00588DCF                 mov     ecx, edi
.text:00588DD1                 mov     esi, eax
.text:00588DD3                 call    _build_cache_file_add_sound_samples

.text:00588E37                 push    1               ; CHANGE THIS TO FALSE
.text:00588E39                 push    ecx
.text:00588E3A                 mov     esi, eax
.text:00588E3C                 call    _build_cache_file_sound_gestalt

.text:00588E99                 push    1               ; CHANGE THIS TO FALSE
.text:00588E9B                 lea     edx, [esp+1F88h+cache_header]
.text:00588EA2                 push    edx
.text:00588EA3                 push    edi
.text:00588EA4                 mov     esi, eax
.text:00588EA6                 call    _build_cache_file_add_geometry_blocks

.text:0058908E                 push    1               ; CHANGE THIS TO FALSE
.text:00589090                 mov     esi, eax
.text:00589092                 call    _language_packing

.text:005890A7                 push    1               ; CHANGE THIS TO FALSE
.text:005890A9                 push    ecx
.text:005890AA                 call    _build_cache_file_add_language_packs

.text:00589105                 push    1               ; CHANGE THIS TO FALSE
.text:00589107                 push    edi
.text:00589108                 mov     esi, eax
.text:0058910A                 call    _build_cache_file_add_bitmap_pixels

.text:00589181                 push    1               ; CHANGE THIS TO FALSE
.text:00589183                 push    1400000h        ; base address
.text:00589188                 push    edx
.text:00589189                 mov     esi, eax
.text:0058918B                 push    edi
.text:0058918C                 lea     eax, [esp+1F94h+multiplayer_shared_dependency_graph]
.text:00589193                 push    eax
.text:00589194                 lea     ecx, [esp+1F98h+cache_header]
.text:0058919B                 push    ecx
.text:0058919C                 mov     ecx, [esp+1F9Ch+multiplayer_shared_tag_index_sorted]
.text:005891A0                 lea     edx, [esp+1F9Ch+scenario_name_wide]
.text:005891A4                 call    _build_cache_file_add_tags ; ecx = s_shared_tag_index

*/

#pragma endregion

void H2ToolPatches::apply_shared_tag_removal_scheme()
{
	BYTE* patching_offsets_list[] = {
		CAST_PTR(BYTE*, 0x5887DF),//_postprocess_animation_data
		CAST_PTR(BYTE*, 0x588C21),//_build_cache_file_prepare_sound_gestalt
		CAST_PTR(BYTE*, 0x588D52),//_build_cache_file_add_tags
		CAST_PTR(BYTE*, 0x588DCD),//_build_cache_file_add_sound_samples
		CAST_PTR(BYTE*, 0x588E37),//_build_cache_file_sound_gestalt
		CAST_PTR(BYTE *,0x588E99),//_build_cache_file_add_geometry_blocks
		CAST_PTR(BYTE *,0x58908E),//_language_packing
		CAST_PTR(BYTE *,0x5890A7),//_build_cache_file_add_language_packs
		CAST_PTR(BYTE *,0x589105),//_build_cache_file_add_bitmap_pixels
		CAST_PTR(BYTE *,0x589181),//_build_cache_file_add_tags
	};

	BYTE patch[1] = { 0x0 };
	for (int x = 0; x < NUMBEROF(patching_offsets_list); x++)			WriteBytes((DWORD)(patching_offsets_list[x] + 1), patch, 1);//patching push 1 -> push 0
}

void H2ToolPatches::unlock_other_scenario_types_compiling()
{
	//Refer to H2EK_OpenSauce Campaign_sharing
	static void* BUILD_CACHE_FILE_FOR_SCENARIO__CHECK_SCENARIO_TYPE = CAST_PTR(void*,0x588320);
	BYTE patch[1] = {0xEB};
	WriteBytes((DWORD)BUILD_CACHE_FILE_FOR_SCENARIO__CHECK_SCENARIO_TYPE, patch, 1);//change jz to jmp

}

static signed long _scenario_type;

static void __declspec(naked) _build_cache_file_for_scenario__intercept_get_scenario_type()
{
	//Refer to H2EK_OpenSauce Campaign_sharing
	//Basically this function helps us to store the scenario_type which can be used in later areas
	static const unsigned __int32 INTERCEPTOR_EXIT = 0x588313;

	__asm {
		    movsx	edx, word ptr[eax + 0x1C]
			push	esi
			mov[esp + 0x58], edx			// mov     [esp+1F90h+scenario_type], edx
			mov		_scenario_type, edx
			jmp		INTERCEPTOR_EXIT
	}
}

const static wchar_t *campaign_shared_path = L"maps\\single_player_shared.map";
const static char *load_sharing_log_messages[] =
{
	"tag sharing: loading tag names from single_player_shared.map",
	"tag sharing: failed to open singleplayer shared map file",
	"singleplayer shared cache file doesn't have its string table!! AAAAaaaaggghh!!!",
	"singleplayer shared cache file string count is suspect (> 0x6000)",
	"singleplayer shared cache file doesn't have its tag dependency graph!! AAAAaaaaggghh!!!",
	"singleplayer shared cache file tag dependency graph size is suspect (> 0x100000)",
	"tag sharing: ruh roh, single_player_shared.map has no shared tags"

};

const static int load_sharing_log_offsets[] =
{
	0x005813E7,
	0x00581499,
	0x005814B7,
	0x005818FF,
	0x00581587,
	0x005818EB,
	0x005817CD
};

static char __cdecl h_BUILD_CACHE_FILE_FOR_SCENARIO__TAG_SHARING_LOAD_SHARED(void *a1, void* a2)
{
	//Refer to H2EK_OpenSauce Campaign_sharing

	// If scenario_type is single_player,modify the strings
	if (_scenario_type == 0)
	{	
		// Replace file name strings
		WritePointer(0x581455, campaign_shared_path);
		WritePointer(0x581480, campaign_shared_path);

		// fix log strings to match
		for (int i = 0; i < ARRAYSIZE(load_sharing_log_offsets); i++)
			WritePointer(load_sharing_log_offsets[i] + 1, load_sharing_log_messages[i]);
	}
	H2PCTool.WriteLog("loading....tag_sharing method");


	DWORD TAG_SHARING_LOAD_MULTIPLAYER_SHARED = 0x5813C0;
	return ((char(__cdecl *)(void*, void*))TAG_SHARING_LOAD_MULTIPLAYER_SHARED)(a1,a2);// call Function via address

}
void H2ToolPatches::render_model_import_unlock()
{
	//Patches the h2tool to use the custom render_model_generation methods

	///Patch Details::#1 patching the orignal render_model_generate function to call mine
	/*
	.text:0041C7A0 000                 mov     eax, [esp+arguments]
	.text:0041C7A4 000                 mov     ecx, [eax]
	.text:0041C7A6 000                 jmp     loc_41C4A0      ; Jump
	*/
	PatchCall(0x41C7A6, h2pc_import_render_model_proc);
}

void H2ToolPatches::enable_campaign_tags_sharing()
{
	//Refer to H2EK_OpenSauce Campaign_sharing

    void* BUILD_CACHE_FILE_FOR_SCENARIO__GET_SCENARIO_TYPE = CAST_PTR(void*, 0x58830A);
	int BUILD_CACHE_FILE_FOR_SCENARIO__TAG_SHARING_LOAD_SHARED =  0x5883DE;

	/*
	//get_scenario_ Intercept codes
	BYTE patch[1] = { 0xE8};

	//Writing a call in memory to jmp to our function
	WriteBytesASM((DWORD)BUILD_CACHE_FILE_FOR_SCENARIO__GET_SCENARIO_TYPE, patch, 1);
	PatchCall((DWORD)BUILD_CACHE_FILE_FOR_SCENARIO__GET_SCENARIO_TYPE, (DWORD)_build_cache_file_for_scenario__intercept_get_scenario_type);//Writing the address to be jumped
	*/
	WriteJmp(BUILD_CACHE_FILE_FOR_SCENARIO__GET_SCENARIO_TYPE, _build_cache_file_for_scenario__intercept_get_scenario_type);


	//single_player_shared sharing
	
	//.text:005883DE                 call    BUILD_CACHE_FILE_FOR_SCENARIO__TAG_SHARING_LOAD_SHARED ; STR: "tag sharing: loading tag names from shared.map", "tag sharing: 
	PatchCall(BUILD_CACHE_FILE_FOR_SCENARIO__TAG_SHARING_LOAD_SHARED, h_BUILD_CACHE_FILE_FOR_SCENARIO__TAG_SHARING_LOAD_SHARED);//modifying the call to go to my h_function rather orignal

	H2PCTool.WriteLog("Single Player tag_sharing enabled");
}

void H2ToolPatches::remove_bsp_version_check()
{
	// allow tool to work with BSPs compliled by newer versions of tool.
	// downgrades the error you would get to a non-fatal one.
	BYTE bsp_version_check_return[] = { 0xB0, 0x01 };
	WriteBytes(0x00545D0F, bsp_version_check_return, sizeof(bsp_version_check_return));
}

void H2ToolPatches::disable_secure_file_locking()
{
	// allow other processes to read files open with fopen_s
	WriteValue(0x0074DDD6 + 1, _SH_DENYWR);
}

hs_convert_data_store *hs_get_converter_data_store(unsigned __int16 handle)
{
	typedef void* (__cdecl *get_args)(int a1, unsigned __int16 a2);
	get_args get_args_impl = reinterpret_cast<get_args>(0x00557E60);
	return static_cast<hs_convert_data_store*>(get_args_impl(*reinterpret_cast<DWORD*>(0xBCBF4C), handle));
}

const char *hs_get_string_data(hs_convert_data_store *data_store)
{
	const char *hs_string_data = *reinterpret_cast<const char **>(0x00CDB198);
	return &hs_string_data[data_store->string_value_offset];
}

char hs_error[0x1024];

void hs_converter_error(hs_convert_data_store *data_store, const std::string &error)
{
	const char **hs_error_string_ptr = reinterpret_cast<const char**>(0x00CDB1AC);
	DWORD *hs_error_offset_ptr = reinterpret_cast<DWORD*>(0x00CDB1B0);

	strncpy(hs_error, error.c_str(), sizeof(hs_error));

	*hs_error_string_ptr = hs_error;
	*hs_error_offset_ptr = data_store->string_value_offset;
	data_store->output = NONE;
}

scnr_tag *get_global_scenario()
{
	return *reinterpret_cast<scnr_tag **>(0x00AA00E4);
}

void hs_convert_string_id_to_tagblock_offset(tag_block_ref *tag_block, int element_size, int block_offset, int hs_converter_id)
{
	auto data_store = hs_get_converter_data_store(hs_converter_id);
	const char *value_string = hs_get_string_data(data_store);

	data_store->output = FIND_TAG_BLOCK_STRING_ID(tag_block, element_size, block_offset, GET_STRING_ID(value_string));

	if (data_store->output == NONE) {
		std::string error = "this is not a valid '" + hs_type_string[static_cast<hs_type>(data_store->target_hs_type)] + "' name, check tags";
		hs_converter_error(data_store, error);
	}
}

void hs_convert_string_to_tagblock_offset(tag_block_ref *tag_block, int element_size, int block_offset, int hs_converter_id)
{
	auto data_store = hs_get_converter_data_store(hs_converter_id);
	const char *value_string = hs_get_string_data(data_store);

	data_store->output = FIND_TAG_BLOCK_STRING(tag_block, element_size, block_offset, value_string);

	if (data_store->output == NONE) {
		std::string error = "this is not a valid '" + hs_type_string[static_cast<hs_type>(data_store->target_hs_type)] + "' name, check tags";
		hs_converter_error(data_store, error);
	}
}

char __cdecl hs_convert_conversation(unsigned __int16 a1)
{
	scnr_tag *scenario = get_global_scenario();
	hs_convert_string_to_tagblock_offset(&scenario->aIConversations, 128, 0, a1);
	return 1;
}

char __cdecl hs_convert_internal_id_passthrough(unsigned __int16 a1)
{
	hs_convert_data_store *data_store = hs_get_converter_data_store(a1);
	const char *input_string = hs_get_string_data(data_store);
	if (!_stricmp(input_string, "NONE")) {
		data_store->output = NONE;
		return 1;
	}
	try {
		data_store->output = std::stoi(input_string, nullptr, 0);
		return 1;
	}
	catch (invalid_argument) {
		hs_converter_error(data_store, "invalid " + get_hs_type_string(data_store->target_hs_type) + " ID");
		return 0;
	}
	catch (out_of_range)
	{
		hs_converter_error(data_store, get_hs_type_string(data_store->target_hs_type) + " ID out of range");
		return 0;
	}
}

char __cdecl hs_convert_ai_behaviour(unsigned __int16 a1)
{
	hs_convert_data_store *data_store = hs_get_converter_data_store(a1);
	const std::string input = hs_get_string_data(data_store);
	ai_behaviour out = string_to_ai_behaviour(input);
	if (out != ai_behaviour::invalid) {
		data_store->output = static_cast<DWORD>(out);
		return 1;
	} else {
		hs_converter_error(data_store, "Invalid AI behaviour");
		return 0;
	}
}

char __cdecl hs_convert_ai_orders(unsigned __int16 a1)
{
	scnr_tag *scenario = get_global_scenario();
	hs_convert_string_to_tagblock_offset(&scenario->orders, 144, 0, a1);
	return 1;
}

enum ai_id_type
{
	squad,
	squad_group,
	unknown,
	starting_location,

	none = NONE
};

char __cdecl hs_convert_ai(unsigned __int16 a1)
{
	scnr_tag *scenario = get_global_scenario();
	hs_convert_data_store *data_store = hs_get_converter_data_store(a1);
	std::string input_string = hs_get_string_data(data_store);
	
	ai_id_type ai_type = ai_id_type::none;
	
	DWORD main_index = NONE;
	DWORD secondary_index = 0;

	if (input_string.find('/') != string::npos) {
		ai_type = ai_id_type::starting_location;
		std::string squad_name = input_string.substr(0, input_string.find('/'));
		std::string squad_pos = input_string.substr(input_string.find('/') + 1);

		secondary_index = FIND_TAG_BLOCK_STRING(&scenario->squads, 120, 0, squad_name);
		if (secondary_index != NONE)
		{
			// can't get the sqaud block struct working so this is a workaround for now
			main_index = strtol(squad_pos.c_str(), nullptr, 0);
		} else {
			hs_converter_error(data_store, "No such squad.");
			return 0;
		}
	} else {

		ai_type = ai_id_type::squad;
		// attempt to find a squad with that name first
		main_index = FIND_TAG_BLOCK_STRING(&scenario->squads, 120, 0, input_string);
		if (main_index == NONE) {
			// if no sqaud with that name exists try the sqaud groups
			ai_type = ai_id_type::squad_group;
			main_index = FIND_TAG_BLOCK_STRING(&scenario->squadGroups, 36, 0, input_string);
		}
	}
	if (main_index != NONE) {
		DWORD id = (ai_type << 30) | (secondary_index << 16) | main_index;
		data_store->output = id;
		return 1;
	} else {
		// fallback to passthrough for backwards compatibility and AI squad starting locations
		return hs_convert_internal_id_passthrough(a1);
	}
}

char __cdecl hs_convert_point_ref(unsigned __int16 a1)
{
	scnr_tag *scenario = get_global_scenario();
	hs_convert_data_store *data_store = hs_get_converter_data_store(a1);
	std::string input_string = hs_get_string_data(data_store);
	auto scripting_data = scenario->scriptingData;

	if (input_string.find('/') != string::npos) {
		std::string point_set = input_string.substr(0, input_string.find('/'));
		std::string point = input_string.substr(input_string.find('/') + 1);

		int point_set_index = FIND_TAG_BLOCK_STRING(&scripting_data.data->pointSets,
			sizeof(cs_point_set_block),
			offsetof(cs_point_set_block, name),
			point_set);
		if (point_set_index != NONE)
		{
			cs_point_set_block *points = &scripting_data.data->pointSets.data[point_set_index];
			int point_index = FIND_TAG_BLOCK_STRING(&points->points,
				sizeof(cs_point_block),
				offsetof(cs_point_block, name),
				point);

			if (point_index != NONE)
			{
				data_store->output = (point_set_index << 16 | point_index);
				return 1;
			} else {
				hs_converter_error(data_store, "No such point.");
				return false;
			}
		} else {
			hs_converter_error(data_store, "No such point set.");
			return false;
		}
	} else {
		hs_converter_error(data_store, "Invalid format.");
		return false;
	}
}

#define set_hs_converter(type, func) \
	hs_convert_lookup_table[static_cast<int>(type)] = func;

void fix_hs_converters()
{
	void **hs_convert_lookup_table = reinterpret_cast<void**>(0x009F0C88);
	set_hs_converter(hs_type::ai_behavior, hs_convert_ai_behaviour);
	set_hs_converter(hs_type::conversation, hs_convert_conversation);
	set_hs_converter(hs_type::ai_orders, hs_convert_ai_orders);
	set_hs_converter(hs_type::ai, hs_convert_ai);
	set_hs_converter(hs_type::point_reference, hs_convert_point_ref);

	// hacky workaround, lets the user directly input the ID it's meant to generate.
	hs_type passthrough_types[] = {
		hs_type::style,           hs_type::hud_message,
		hs_type::navpoint,        hs_type::ai_command_list
	};

	for (auto i : passthrough_types)
		set_hs_converter(i, hs_convert_internal_id_passthrough);
}

#undef set_hs_converter

void HaloScriptExtend()
{

	hs_command **command_table = reinterpret_cast<hs_command **>(0x009ECFE0);
	hs_global_variable **global_table = reinterpret_cast<hs_global_variable **>(0x009EFF78);
	g_halo_script_interface->init_custom(command_table, global_table);

	// Replace pointers to the commmand table
	static DWORD cmd_offsets[] =
	{
		0x005C5365 + 3, 0x005C5530 + 3, 0x005C5554 + 3,
		0x005C5821 + 3, 0x005C5C44 + 3, 0x005C5E64 + 3,
		0x005C5E9A + 3, 0x005C5F48 + 3
	};

	hs_command **cmds = g_halo_script_interface->get_command_table();

	for (DWORD addr : cmd_offsets)
		WritePointer(addr, cmds);

	// patch command table size
	const static int hs_cmd_table_size = g_halo_script_interface->get_command_table_size();
	WriteValue(0x008CD59C, hs_cmd_table_size);

	// Replace pointers to the globals table
	static DWORD var_offsets[] =
	{
		0x005C53D5, 0x005C53F0, 0x005C5430,
		0x005C5474, 0x005C58D1, 0x006884A1,
		0x006884BD, 0x0068850D, 0x0068858B,
		0x006885A2
	};

	hs_global_variable **vars = g_halo_script_interface->get_global_table();

	for (DWORD addr : var_offsets)
		WritePointer(addr + 3, vars);

	// patch globals table size
	const static int hs_global_table_size = g_halo_script_interface->get_global_table_size();
	WriteValue(0x008D2238, hs_global_table_size);
}

LPWSTR __crtGetCommandLineW_hook()
{
	typedef LPWSTR (*__crtGetCommandLineW_t)();
	__crtGetCommandLineW_t __crtGetCommandLineW_impl = reinterpret_cast<__crtGetCommandLineW_t>(0x00764EB3);

	wchar_t *real_cmd = __crtGetCommandLineW_impl();
	std::wstring fake_cmd = std::regex_replace(real_cmd, std::wregex(L"( pause_after_run| shared_tag_removal)"), L"");
	wcscpy(real_cmd, fake_cmd.c_str());
	return real_cmd;
}

void H2ToolPatches::fix_command_line()
{
	PatchCall(0x00751F83, __crtGetCommandLineW_hook);
}

void tag_dump(int tag_index)
{
	char old_name[0x200];
	strncpy_s(old_name, TAG_GET_NAME(tag_index), sizeof(old_name));

	std::string new_name = "dump\\";
	new_name += old_name;

	printf("dumping tag '%s' as '%s' ***\n", old_name, new_name.c_str());

	TAG_RENAME(tag_index, new_name);
	TAG_SAVE(tag_index);
	TAG_RENAME(tag_index, old_name);
}

char __cdecl scenario_write_patch_file_hook(int TAG_INDEX, int a2)
{
	typedef char (__cdecl *scenario_write_patch_file)(int TAG_INDEX, int a2);
	auto scenario_write_patch_file_impl = reinterpret_cast<scenario_write_patch_file>(0x0056A110);

	tag_dump(TAG_INDEX);

	scnr_tag *scenario = (scnr_tag*)TAG_GET('scnr', TAG_INDEX);
	for (int i = 0; i < scenario->structureBSPs.size; i++)
		tag_dump(scenario->structureBSPs.data[i].structureBSP.tag_index);

	return scenario_write_patch_file_impl(TAG_INDEX, a2);
}

void H2ToolPatches::Initialize()
{
	H2PCTool.WriteLog("Dll Successfully Injected to H2Tool");
	wcout << "H2Toolz version: " << version << std::endl
		 << "Built on " __DATE__ " at " __TIME__ << std::endl;

	Increase_structure_import_size_Check();
	Increase_structure_bsp_geometry_check();
	AddExtraCommands();
	unlock_other_scenario_types_compiling();
	render_model_import_unlock();
	remove_bsp_version_check();
	disable_secure_file_locking();
	fix_hs_converters();
	HaloScriptExtend();
	fix_command_line();
	//enable_campaign_tags_sharing(); // Still crashes might need tag changes.

	std::string cmd = GetCommandLineA();
	if (cmd.find("shared_tag_removal") != string::npos)
		apply_shared_tag_removal_scheme();

	if (conf.getBoolean("dump_tags_packaging", false))
		PatchCall(0x00588A66, scenario_write_patch_file_hook);
}


void H2ToolPatches::AddExtraCommands()
{
	H2PCTool.WriteLog("Adding Extra Commands to H2Tool");
#pragma region New Function Defination Creation 

	static const BYTE k_number_of_tool_commands = 0xC;
	static const BYTE k_number_of_tool_commands_new = k_number_of_tool_commands + NUMBEROF(h2tool_extra_commands);

	// Tool's original tool commands
	static const s_tool_command* const* tool_import_classes = CAST_PTR(s_tool_command**, 0x97B6EC);
	// The new tool commands list which is made up of tool's
	// and [yelo_extra_tool_commands]
	static s_tool_command* tool_commands[k_number_of_tool_commands_new];

	// copy official tool commands
	memcpy_s(tool_commands, sizeof(tool_commands),
		tool_import_classes, sizeof(s_tool_command*) * k_number_of_tool_commands);
	// copy yelo tool commands
	memcpy_s(&tool_commands[k_number_of_tool_commands], sizeof(h2tool_extra_commands),
		h2tool_extra_commands, sizeof(h2tool_extra_commands));

	// Now I know my ABCs
	qsort_s(tool_commands, NUMBEROF(tool_commands), sizeof(s_tool_command*), s_tool_command_compare, NULL);
#pragma endregion
#pragma region UpdateTool_function References

	DWORD tool_commands_references[] = {
		 0x410596,
		 0x41060E,
		 0x412D86,
	};
    DWORD tool_commands_count[] = {
		 0x4105E5,
		 0x412D99,
	};

	// update references to the tool command definitions
	for (int x = 0; x < NUMBEROF(tool_commands_references); x++)
		WriteValue(tool_commands_references[x], tool_commands);
	


#pragma endregion
#pragma region UpdateTool_functionCount References
	// update code which contain the tool command definitions count

	for (int x = 0; x < NUMBEROF(tool_commands_count); x++)
		WriteValue(tool_commands_count[x], k_number_of_tool_commands_new);

#pragma endregion
}

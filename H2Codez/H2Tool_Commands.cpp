#include "stdafx.h"
#include "H2Tool\ToolCommandDefinitions.inl"
#include "H2Tool\H2Tool_extra_commands.inl"
#include "H2ToolsCommon.h"
#include "Patches.h"
#include "Version.h"

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
	DWORD collision_surfaces_count = 0x7FFF * TOOL_INCREASE_FACTOR; ///1048576
	BYTE* f = reverse_addr(CAST_PTR(void*, collision_surfaces_count));
	BYTE collision_surfaces_count_Patch[4] = { f[0],f[1],f[2],f[3] };


	//increasing the edges_vertices_count
	DWORD edges_vertices_count = 0xFFFF * TOOL_INCREASE_FACTOR; ///2097120
	f = reverse_addr(CAST_PTR(void*, edges_vertices_count));
	BYTE edges_vertices_count_Patch[4] = { f[0],f[1],f[2],f[3] };

	//Patching in Memory
	WriteBytes(0x5A2D50 + 4, collision_surfaces_count_Patch, 4);
	WriteBytes(0x5A2D5A + 1, edges_vertices_count_Patch, 4);

	///Also Patching in the error_proc method incase we ever hit this Limit :)
/*
.text:00464C50 118 push    0FFFFh
.text:00464C55 11C push    eax
.text:00464C56 120 push    0FFFFh
.text:00464C5B 124 push    ecx
.text:00464C5C 128 push    7FFFh
*/
	WriteBytes(0x464C50 + 1, edges_vertices_count_Patch, 4);
	WriteBytes(0x464C56 + 1, edges_vertices_count_Patch, 4);
	WriteBytes(0x464C5C + 1, collision_surfaces_count_Patch, 4);


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
	DWORD new_planes_count = 0x8000 *TOOL_INCREASE_FACTOR; ///1048576
	BYTE* f = reverse_addr(CAST_PTR(void*, new_planes_count));
	BYTE count_Patch[4] = { f[0],f[1],f[2],f[3] };
	WriteBytes(0x5A2CFB+4, count_Patch, 4);


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

void H2ToolPatches::Increase_structure_bsp_geometry_check()
{
	H2PCTool.WriteLog("Increasing structure_bsp_geometry checks");
	structure_bsp_geometry_2D_check_increase();
	structure_bsp_geometry_3D_check_increase();
	structure_bsp_geometry_collision_check_increase();
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

static wcstring k_campaign_shared_name = L"maps\\single_player_shared.map";
static signed long _scenario_type;

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
static void _cdecl _build_cache_file_for_scenario__intercept_get_scenario_type()
{
	//Refer to H2EK_OpenSauce Campaign_sharing
	//Basically this function helps us to store the scenario_type which can be used in later areas
	static const unsigned __int32 INTERCEPTOR_EXIT = 0x588313;
	
	H2PCTool.WriteLog("intercepting _get_scenario_type()");

	__asm {
		movsx	edx, word ptr[eax + 0x1C]
			push	esi
			mov[esp + 0x58], edx			// mov     [esp+1F90h+scenario_type], edx
			mov		_scenario_type, edx
			jmp		INTERCEPTOR_EXIT
	}
}

static char __cdecl h_BUILD_CACHE_FILE_FOR_SCENARIO__TAG_SHARING_LOAD_SHARED(void *a1, void* a2)
{
	//Refer to H2EK_OpenSauce Campaign_sharing

	//If scenario_type is single_player,modify the strings
	if (_scenario_type == 0)
	{		
		
	   DWORD TAG_SHARING_LOAD_SHARED_NAME1 = 0x581455;
	   DWORD TAG_SHARING_LOAD_SHARED_NAME2 = 0x581480;

		
		BYTE *f = reverse_addr((void*)k_campaign_shared_name);
		BYTE k_name_ptr_patch[4] = { f[0],f[1],f[2],f[3] };
		
	    //replacing shared.map text with single_player_shared.map text
		WriteBytes((DWORD)(TAG_SHARING_LOAD_SHARED_NAME1), k_name_ptr_patch, sizeof(k_name_ptr_patch));
		WriteBytes((DWORD)(TAG_SHARING_LOAD_SHARED_NAME2), k_name_ptr_patch, sizeof(k_name_ptr_patch));
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
	void* BUILD_CACHE_FILE_FOR_SCENARIO__TAG_SHARING_LOAD_SHARED = CAST_PTR(void*, 0x5883DE);

	/*
	//get_scenario_ Intercept codes
	BYTE patch[1] = { 0xE8};

	//Writing a call in memory to jmp to our function
	WriteBytesASM((DWORD)BUILD_CACHE_FILE_FOR_SCENARIO__GET_SCENARIO_TYPE, patch, 1);
	PatchCall((DWORD)BUILD_CACHE_FILE_FOR_SCENARIO__GET_SCENARIO_TYPE, (DWORD)_build_cache_file_for_scenario__intercept_get_scenario_type);//Writing the address to be jumped
	*/
	WriteJmpTo(BUILD_CACHE_FILE_FOR_SCENARIO__GET_SCENARIO_TYPE, _build_cache_file_for_scenario__intercept_get_scenario_type);


	//single_player_shared sharing
	
	//.text:005883DE                 call    BUILD_CACHE_FILE_FOR_SCENARIO__TAG_SHARING_LOAD_SHARED ; STR: "tag sharing: loading tag names from shared.map", "tag sharing: 
	PatchCall((DWORD)BUILD_CACHE_FILE_FOR_SCENARIO__TAG_SHARING_LOAD_SHARED, (DWORD)h_BUILD_CACHE_FILE_FOR_SCENARIO__TAG_SHARING_LOAD_SHARED);//modifying the call to go to my h_function rather orignal

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

void hs_converter_error(hs_convert_data_store *data_store, const char *error)
{
	const char **hs_error_string_ptr = reinterpret_cast<const char**>(0x00CDB1AC);
	DWORD *hs_error_offset_ptr = reinterpret_cast<DWORD*>(0x00CDB1B0);

	*hs_error_string_ptr = error;
	*hs_error_offset_ptr = data_store->string_value_offset;
	data_store->output = -1;
}

char __cdecl hs_convert_ai_id(unsigned __int16 a1)
{
	hs_convert_data_store *data_store = hs_get_converter_data_store(a1);
	const char *input_string = hs_get_string_data(data_store);
	try {
		data_store->output = std::stoi(input_string, nullptr, 0);
		return 1;
	}
	catch (invalid_argument) {
		hs_converter_error(data_store, "invalid AI ID");
		return 0;
	}
	catch (out_of_range)
	{
		hs_converter_error(data_store, "AI ID out of range");
		return 0;
	}
}


void H2ToolPatches::Initialize()
{
	H2PCTool.WriteLog("Dll Successfully Injected to H2Tool");
	cout << "H2Toolz version: " << version << std::endl
		 << "Built on " __DATE__ " at " __TIME__ << std::endl;

	Increase_structure_import_size_Check();
	Increase_structure_bsp_geometry_check();
	AddExtraCommands();
	unlock_other_scenario_types_compiling();
	render_model_import_unlock();
	remove_bsp_version_check();
	disable_secure_file_locking();
	//enable_campaign_tags_sharing(); //Crashes H2tool ,maybe we need to update BIN files for Campaign Sharing

	// dirty and horrible AI hack
	void **hs_convert_lookup_table = reinterpret_cast<void**>(0x009F0C88);
	hs_convert_lookup_table[static_cast<int>(hs_type::ai)] = hs_convert_ai_id;

	std::string cmd = GetCommandLineA();
	if (cmd.find("shared_tag_removal") != string::npos)
		apply_shared_tag_removal_scheme();
	

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

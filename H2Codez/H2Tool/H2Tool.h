#pragma once
#include "stdafx.h"
#include "Common\BlamBaseTypes.h"
#include "Common\FiloInterface.h"
#include "CacheBuilder.h"

enum tool_command_argument_type : long {
	_tool_command_argument_type_0,
	_tool_command_argument_type_1,
	_tool_command_argument_type_data_directory,
	_tool_command_argument_type_3,
	_tool_command_argument_type_tag_name,
	_tool_command_argument_type_5,
	_tool_command_argument_type_data_file,
	_tool_command_argument_type_file,			// file system path, not limited to data or tags
	_tool_command_argument_type_real,
	_tool_command_argument_type_string,
	_tool_command_argument_type_10,
	_tool_command_argument_type_radio,			// definition holds choices, separated by '|'
	_tool_command_argument_type_boolean,
	_tool_command_argument_type,
};

struct s_tool_command_argument {
	signed long 	type;
	wcstring        name;
	const char*		definition;
	const char*		help;
};

struct s_tool_command {
	wcstring       					name;
	_tool_command_proc				import_proc;
	const s_tool_command_argument*	arguments;
	int							    argument_count;
	bool							dont_initialize_game;
	unsigned char : 8; unsigned short : 16;
};

typedef bool __cdecl import_def_translate_path(file_reference *data_file, file_reference *tag);

struct s_tool_import_definations
{
	cstring							file_extension;
	_tool_import__defination_proc   import_proc;
	DWORD                           unk_1;
	import_def_translate_path*      translate_path;
};

struct s_tool_import_definations_const
{
	cstring							file_extension;
	uint32_t                        import_proc;
	DWORD                           unk_1;
	uint32_t                        translate_path;
};

typedef bool(_cdecl* tool_dev_command_proc)(const char *name, datum tag);
struct s_tool_h2dev_command {
	cstring name;
	cstring description;
	blam_tag tag_type;
	// not used in any working commands
	blam_tag convert_from;
	// always 1 in working commands
	DWORD arg_type__maybe; 
	enum _flag
	{
		bit0 = FLAG(0),
		bit1 = FLAG(1),
	} flags;
	tool_dev_command_proc command_impl;
};
static_assert(sizeof(s_tool_h2dev_command) == 0x1C, "Invalid struct size for dev_command");

namespace H2ToolPatches
{
	void Initialize();

	void fix_command_line();

	inline CacheBuilder::cache_builder_state *get_build_cache_file_globals()
	{
		return reinterpret_cast<CacheBuilder::cache_builder_state*>(0xC48A68);
	}

	void AddExtraCommands();

	void enable_campaign_tags_sharing();
	void apply_shared_tag_removal_scheme();

	void unlock_other_scenario_types_compiling();
	void render_model_import_unlock();
	void remove_bsp_version_check();

	void Increase_structure_import_size_Check();
	void Increase_structure_bsp_geometry_check();

	void structure_bsp_geometry_2D_check_increase();
	void structure_bsp_geometry_3D_check_increase();
	void structure_bsp_geometry_collision_check_increase();

	void disable_secure_file_locking();

	void patch_cache_writter();

	// fix code for distributing light mapping over multiple computers
	void reenable_lightmap_farming();

	// fix bitmap error messages
	void fix_bitmap_package();

	// fix animations import code
	void fix_import_animations();
};

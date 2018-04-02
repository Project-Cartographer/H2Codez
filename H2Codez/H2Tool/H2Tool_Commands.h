#pragma once
#include "stdafx.h"

void _H2ToolAttachHooks();
void _H2ToolDetachHooks();




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
struct s_h2ek_hs_table_commands
{

};
struct s_tool_import_definations_
{
	cstring							file_extension;
	_tool_import__defination_proc   import_proc;
	DWORD                           unk_1;
	DWORD                           unk_2;
};
struct s_tool_h2dev_command {
	cstring command_name;
	cstring command_description;
	DWORD tag_type; // or char[4] in Little-endian / 4 character constant
	DWORD					    unk;
	DWORD					    unk_2;
	DWORD                       unk_3;
	tool_dev_command_proc		command_impl;
};
static_assert(sizeof(s_tool_h2dev_command) == 0x1C, "Invalid struct size for dev_command");

class H2ToolPatches
{
public:
	static void Initialize();

private:
	static void AddExtraCommands();

	static void enable_campaign_tags_sharing();
	static void apply_shared_tag_removal_scheme();

	static void unlock_other_scenario_types_compiling();
	static void render_model_import_unlock();
	static void remove_bsp_version_check();

	static void Increase_structure_import_size_Check();
	static void Increase_structure_bsp_geometry_check();

	static void structure_bsp_geometry_2D_check_increase();
	static void structure_bsp_geometry_3D_check_increase();
	static void structure_bsp_geometry_collision_check_increase();

	static void disable_secure_file_locking();
};




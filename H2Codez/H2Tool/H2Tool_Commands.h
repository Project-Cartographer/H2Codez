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
struct s_tool_h2dev_command {
	const char*       			    name;
	const char*                     description;
	SHORT                           tag_type;		
	DWORD							unk;
	DWORD							condition;
	DWORD                           parameters_count;
	_tool_command_proc				call_proc;
};
struct s_file_reference
{
	unsigned long			signature;
	unsigned short      	flags;
	signed short     		location;
	long_string	            file_name;
	HANDLE		            handle;
	HRESULT		            api_result;
}; 
BOOST_STATIC_ASSERT(sizeof(s_file_reference) == 0x110);

class H2Tool_Extras
{
public:
	void Initialize();
	void AddExtraCommands();
	void Increase_structure_import_size_Check();
	void Increase_structure_bsp_geometry_check();
	void unlock_other_scenario_types_compiling();
	void enable_campaign_tags_sharing();
	void apply_shared_tag_removal_scheme();


private:
	void structure_bsp_geometry_2D_check_increase();
	void structure_bsp_geometry_3D_check_increase();
	void structure_bsp_geometry_collision_check_increase();
};




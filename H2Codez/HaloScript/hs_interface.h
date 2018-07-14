#pragma once
#include "../stdafx.h"
#include "hs_ai_behaviour.h"
#include "hs_types.h"

#include "hs_command.h"
#include "hs_opcodes.h"

#include "hs_global_variable.h"
#include "hs_global_ids.h"

#include <string>

namespace HaloScriptCommon
{
	bool hs_execute(char *script, bool ran_from_console = true);

	/* Used to return data to the scripting engine, marks the end of a script command*/
	void **epilog(void *DatumIndex, int return_data);

	/* returns the arguments passed to the command */
	void **prolog(__int16 command_id, void *DatumIndex, char user_cmd);

	char __cdecl hs_default_func_check(__int16 opcode, void *DatumIndex);

	std::string get_value_as_string(void *var_ptr, hs_type type);
}

using namespace HaloScriptCommon;

class HaloScriptInterface
{
public:

	void init_custom(hs_command **old_command_table, hs_global_variable **global_table);

	inline int get_command_table_size() {
		return static_cast<int>(hs_opcode::enum_count);
	}

	inline int get_global_table_size() {
		return static_cast<int>(hs_global_id::enum_count);
	}

	inline void RegisterCommand(hs_opcode id, hs_command *cmd)
	{
		command_table[static_cast<int>(id)] = cmd;
	}

	inline void RegisterGlobal(hs_global_id id, hs_global_variable *var)
	{
		global_table[static_cast<int>(id)] = var;
	}

	inline hs_command **get_command_table()
	{
		return command_table;
	}

	inline hs_global_variable **get_global_table()
	{
		return global_table;
	}

	hs_command *command_table[static_cast<int>(hs_opcode::enum_count)];

	hs_global_variable *global_table[static_cast<int>(hs_global_id::enum_count)];
};

extern HaloScriptInterface *g_halo_script_interface;
#pragma once
#include "../stdafx.h"
#include "hs_ai_behaviour.h"
#include "hs_types.h"

#include "hs_command.h"
#include "hs_opcodes.h"

#include "hs_global_variable.h"
#include "hs_global_ids.h"

#include "hs_syntax_node.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace HaloScriptCommon
{
	/* Compiles and executes the HaloScript data */
	bool hs_runtime_execute(const char *script, bool ran_from_console = true);

	/* Used to return data to the scripting engine, marks the end of a script command*/
	void **hs_return(datum thread_id, unsigned int return_data);

	/* returns the arguments passed to the command */
	void *hs_get_args(__int16 command_id, datum thread_id, char user_cmd);

	char __cdecl hs_default_func_parse(__int16 opcode, datum thread_id);

	std::string get_value_as_string(const void *var_ptr, hs_type type);

	typedef unsigned int(*custom_hs_func)(void *data);
	struct hs_custom_command
	{
		std::string name;
		custom_hs_func command_impl;
		std::vector<hs_type> args;
		hs_type return_type;
		std::string description;
		std::string custom_usage;
		hs_custom_command(const std::string &_name,
			const std::string &_description,
			custom_hs_func _command_impl,
			const std::vector<hs_type> &_args = {},
			hs_type _return_type = hs_type::nothing,
			const std::string &_custom_usage = "") :
			name(_name),
			command_impl(_command_impl),
			args(_args),
			return_type(_return_type),
			description(_description),
			custom_usage(_custom_usage)
		{
		}

	};
}

using namespace HaloScriptCommon;
class HaloScriptInterface
{
public:

	void init_custom(hs_command **old_command_table, hs_global_variable **global_table);

	static constexpr size_t get_command_table_count() {
		return static_cast<int>(hs_opcode::enum_count);
	}

	static constexpr size_t get_global_table_count() {
		return static_cast<int>(hs_global_id::enum_count);
	}

	inline void RegisterCommand(hs_opcode id, const hs_command *cmd)
	{
		command_table[static_cast<int>(id)] = cmd;
	}

	void RegisterCustomCommand(hs_opcode id, const hs_custom_command &custom_cmd);

	inline void RegisterGlobal(hs_global_id id, const hs_global_variable *var)
	{
		global_table[static_cast<int>(id)] = var;
	}

	inline const hs_command **get_command_table()
	{
		return command_table;
	}

	inline const hs_global_variable **get_global_table()
	{
		return global_table;
	}

	inline custom_hs_func get_custom_func(hs_opcode id)
	{
		auto ilter = custom_funcs.find(id);
		if (ilter != custom_funcs.end())
		{
			return ilter->second;
		}
		return nullptr;
	}

	inline const hs_command *get_command(hs_type id)
	{
		return command_table[static_cast<int>(id)];
	}

	inline void set_command(hs_type id, const hs_command *cmd)
	{
		command_table[static_cast<int>(id)] = cmd;
	}

	inline const hs_global_variable *get_global(hs_global_id id)
	{
		return global_table[static_cast<int>(id)];
	}

	inline void set_global(hs_global_id id, const hs_global_variable *global)
	{
		global_table[static_cast<int>(id)] = global;
	}

	const hs_command *command_table[static_cast<int>(hs_opcode::enum_count)];

	const hs_global_variable *global_table[static_cast<int>(hs_global_id::enum_count)];

private:

	std::unordered_map<hs_opcode, custom_hs_func> custom_funcs;
};

#define HS_FUNC(exper) [](void *args) -> unsigned int { exper }
#define NULL_HS_FUNC HS_FUNC(return 0;)
extern HaloScriptInterface *g_halo_script_interface;
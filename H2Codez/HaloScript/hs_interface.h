#pragma once
#include "../stdafx.h"
#include "hs_ai_behaviour.h"
#include "hs_types.h"

#include "hs_command.h"
#include "hs_opcodes.h"

#include "hs_global_variable.h"
#include "hs_global_ids.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace HaloScriptCommon
{
	bool hs_execute(const char *script, bool ran_from_console = true);

	/* Used to return data to the scripting engine, marks the end of a script command*/
	void **epilog(datum thread_id, int return_data);

	/* returns the arguments passed to the command */
	void *prolog(__int16 command_id, datum thread_id, char user_cmd);

	char __cdecl hs_default_func_check(__int16 opcode, datum thread_id);

	std::string get_value_as_string(const void *var_ptr, hs_type type);

	typedef int(*custom_hs_func)(void *data);
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

	constexpr int get_command_table_size() const {
		return static_cast<int>(hs_opcode::enum_count);
	}

	constexpr int get_global_table_size() const {
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

	const hs_command *command_table[static_cast<int>(hs_opcode::enum_count)];

	const hs_global_variable *global_table[static_cast<int>(hs_global_id::enum_count)];

private:

	std::unordered_map<hs_opcode, custom_hs_func> custom_funcs;
};

#define HS_FUNC(exper) [](void *args) ->int { exper }
#define NULL_HS_FUNC HS_FUNC(return 0;)
extern HaloScriptInterface *g_halo_script_interface;
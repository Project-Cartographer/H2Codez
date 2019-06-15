#include "hs_interface.h"
#include "../h2codez.h"
#include "util/array.h"

HaloScriptInterface script_interface;
HaloScriptInterface *g_halo_script_interface = &script_interface;

bool HaloScriptCommon::hs_runtime_execute(const char *script, bool ran_from_console)
{
	typedef char (__cdecl *c_hs_execute)(const char *script, int ran_from_console);

	int hs_execute_string_ptr = SwitchAddessByMode(0x005C6440, 0x004E35F0, 0x004D5790);
	CHECK_FUNCTION_SUPPORT(hs_execute_string_ptr);

	c_hs_execute c_hs_execute_impl = reinterpret_cast<c_hs_execute>(hs_execute_string_ptr);
	return c_hs_execute_impl(script, ran_from_console);
}

void **HaloScriptCommon::hs_return(datum thread_id, unsigned int return_data)
{
	typedef void **(__cdecl *hs_epilog)(datum thread_id, unsigned int return_data);
	hs_epilog hs_epilog_impl = reinterpret_cast<hs_epilog>(SwitchAddessByMode(0, 0x52CC70, 0));
	CHECK_FUNCTION_SUPPORT(hs_epilog_impl);

	return hs_epilog_impl(thread_id, return_data);
}

void *HaloScriptCommon::hs_get_args(__int16 command_id, datum thread_id, char user_cmd)
{
	typedef void *(__cdecl *hs_prolog)(__int16 a1, datum thread_id, char a3);
	hs_prolog hs_prolog_impl = reinterpret_cast<hs_prolog>(SwitchAddessByMode(0, 0x52D3E0,0 ));
	CHECK_FUNCTION_SUPPORT(hs_prolog_impl);

	return hs_prolog_impl(command_id, thread_id, user_cmd);
}

char __cdecl HaloScriptCommon::hs_default_func_parse(__int16 opcode, datum thread_id)
{
	int hs_default_check = SwitchAddessByMode(0x65F090, 0x581EB0, 0x56E910);
	CHECK_FUNCTION_SUPPORT(hs_default_check);

	func_parse hs_default_check_impl = reinterpret_cast<func_parse>(hs_default_check);
	return hs_default_check_impl(opcode, thread_id);
}

std::string HaloScriptCommon::get_value_as_string(const void *data, hs_type type)
{
	const static bool false_hack = false;
	std::string value_as_string;
	if (!data)
		data = &false_hack;

	switch (type) {
	case hs_type::boolean:
		value_as_string = (*static_cast<const bool*>(data)) ? "True" : "False";
		return value_as_string;
	case hs_type::hs_long:
		value_as_string = std::to_string(*static_cast<const long*>(data));
		return value_as_string;
	case hs_type::nothing:
		return "void";;
	case hs_type::real:
		value_as_string = std::to_string(*static_cast<const float*>(data));
		return value_as_string;
	case hs_type::hs_short:
		value_as_string = std::to_string(*static_cast<const short*>(data));
		return value_as_string;
	default: {
		std::string error_message = "Converting HS data of type '" + hs_type_string[type] + "' to string is not implemented.\n";
		pLog.WriteLog(error_message.c_str());
		value_as_string = std::to_string(*static_cast<const long*>(data));
		return value_as_string;
	}
	}
}

constexpr static hs_global_variable fake_var{ "\x1fpad", hs_type::nothing,  nullptr };

/*
	op_codes and global_ids are different in h2ek and the game, h2codez fixes that.
	This function needs to translate the ids used in the built-in table to the ones that h2codez and game use.
*/

hs_global_id fake_ids[] = {
		hs_global_id::fake_310, hs_global_id::fake_311, hs_global_id::fake_312, hs_global_id::fake_313,
		hs_global_id::fake_598, hs_global_id::fake_599, hs_global_id::fake_600, hs_global_id::fake_601, hs_global_id::fake_602, hs_global_id::fake_603, hs_global_id::fake_604, hs_global_id::fake_605, hs_global_id::fake_606
};
void HaloScriptInterface::init_custom(hs_command **old_command_table, hs_global_variable **old_global_table)
{

	size_t old_table_offset = 0;
	size_t new_table_offset = 0;
	while (new_table_offset <= static_cast<int>(hs_opcode::disable_render_light_suppressor)) {
		if (old_table_offset == 905) // extra command in old table (clan_data_cache_flush)
			old_table_offset++;
		if (new_table_offset == static_cast<int>(hs_opcode::hs_unk_1))
			new_table_offset += 4;
		command_table[new_table_offset++] = old_command_table[old_table_offset++];
	}

	old_table_offset = 0;
	new_table_offset = 0;
	while (new_table_offset <= static_cast<int>(hs_global_id::force_crash_uploads)) {
		if (array_util::contains(fake_ids, static_cast<hs_global_id>(new_table_offset)))
		{
			global_table[new_table_offset++] = &fake_var;
			continue;
		}
		if (new_table_offset == static_cast<int>(hs_global_id::some_radar_thing))
			new_table_offset++;
		global_table[new_table_offset++] = old_global_table[old_table_offset++];
	}

	//  init the rest of the table
	for (size_t i = static_cast<size_t>(hs_global_id::end_api_extension); i < static_cast<size_t>(hs_global_id::enum_count); i++)
	{
		global_table[i] = &fake_var;
	}
}

void __cdecl custom_func_wrapper(__int16 opcode, datum thread_id, char user_cmd)
{
	hs_opcode id = static_cast<hs_opcode>(opcode);
	auto custom_func = g_halo_script_interface->get_custom_func(id);
	if (LOG_CHECK(id < hs_opcode::enum_count) && LOG_CHECK(custom_func))
	{
		auto cmd = g_halo_script_interface->command_table[opcode];
		void *args = nullptr;
		if (cmd->arg_count > 0)
			args = HaloScriptCommon::hs_get_args(opcode, thread_id, user_cmd);
		if (args || cmd->arg_count == 0)
		{
			HaloScriptCommon::hs_return(thread_id, custom_func(args));
		}
	} else {
		HaloScriptCommon::hs_return(thread_id, 0);
	}
}

// not freed till the process exists and the OS cleans up
const char *cmd_string(const std::string &str)
{
	if (!str.empty())
	{
		size_t data_size = str.size() + 1;
		char *data = new char[data_size];
		strncpy_s(data, data_size, str.c_str(), str.size());
		return static_cast<const char*>(data);
	}
	return nullptr;
}

void HaloScriptInterface::RegisterCustomCommand(hs_opcode id, const hs_custom_command &custom_cmd)
{
	custom_funcs[id] = custom_cmd.command_impl;
	hs_command *command = NewCommand(
		cmd_string(custom_cmd.name),
		custom_cmd.return_type,
		(func_parse)SwitchAddessByMode(0x65F090, 0x581EB0, 0x56E910),
		custom_func_wrapper,
		cmd_string(custom_cmd.description),
		cmd_string(custom_cmd.custom_usage),
		custom_cmd.args.size(),
		custom_cmd.args.data()
	);
	RegisterCommand(id, command);
}
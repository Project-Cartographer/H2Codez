#include "hs_interface.h"
#include "../h2codez.h"

HaloScriptInterface script_interface;
HaloScriptInterface *g_halo_script_interface = &script_interface;

bool HaloScriptCommon::hs_execute(const char *script, bool ran_from_console)
{
	typedef char (__cdecl *c_hs_execute)(const char *script, int ran_from_console);

	int hs_execute_string_ptr = SwitchAddessByMode(0x005C6440, 0x004E35F0, 0x004D5790);
	CHECK_FUNCTION_SUPPORT(hs_execute_string_ptr);

	c_hs_execute c_hs_execute_impl = reinterpret_cast<c_hs_execute>(hs_execute_string_ptr);
	return c_hs_execute_impl(script, ran_from_console);
}

void **HaloScriptCommon::epilog(datum thread_id, int return_data)
{
	typedef void **(__cdecl *hs_epilog)(datum thread_id, int return_data);
	hs_epilog hs_epilog_impl = reinterpret_cast<hs_epilog>(SwitchAddessByMode(0, 0x52CC70, 0));
	CHECK_FUNCTION_SUPPORT(hs_epilog_impl);

	return hs_epilog_impl(thread_id, return_data);
}

void *HaloScriptCommon::prolog(__int16 command_id, datum thread_id, char user_cmd)
{
	typedef void *(__cdecl *hs_prolog)(__int16 a1, datum thread_id, char a3);
	hs_prolog hs_prolog_impl = reinterpret_cast<hs_prolog>(SwitchAddessByMode(0, 0x52D3E0,0 ));
	CHECK_FUNCTION_SUPPORT(hs_prolog_impl);

	return hs_prolog_impl(command_id, thread_id, user_cmd);
}

char __cdecl HaloScriptCommon::hs_default_func_check(__int16 opcode, datum thread_id)
{
	int hs_default_check = SwitchAddessByMode(0x65F090, 0x581EB0, 0x56E910);
	CHECK_FUNCTION_SUPPORT(hs_default_check);

	func_check hs_default_check_impl = reinterpret_cast<func_check>(hs_default_check);
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


void HaloScriptInterface::init_custom(hs_command **old_command_table, hs_global_variable **old_global_table)
{

	int old_table_offset = 0;
	int new_table_offset = 0;
	while (new_table_offset <= static_cast<int>(hs_opcode::disable_render_light_suppressor)) {
		if (old_table_offset == 905) // extra command in old table (clan_data_cache_flush)
			old_table_offset++;
		if (new_table_offset == static_cast<int>(hs_opcode::hs_unk_1))
			new_table_offset += 4;
		command_table[new_table_offset] = old_command_table[old_table_offset];
		old_table_offset++;
		new_table_offset++;
	}

	for (unsigned int i = 0; i <= static_cast<int>(hs_global_id::force_crash_uploads); i++) {
		global_table[i] = old_global_table[i];
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
			args = HaloScriptCommon::prolog(opcode, thread_id, user_cmd);
		if (args || cmd->arg_count == 0)
		{
			HaloScriptCommon::epilog(thread_id, custom_func(args));
		}
	} else {
		HaloScriptCommon::epilog(thread_id, 0);
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
		(func_check)SwitchAddessByMode(0x65F090, 0x581EB0, 0x56E910),
		custom_func_wrapper,
		cmd_string(custom_cmd.description),
		cmd_string(custom_cmd.custom_usage),
		custom_cmd.args.size(),
		custom_cmd.args.data()
	);
	RegisterCommand(id, command);
}
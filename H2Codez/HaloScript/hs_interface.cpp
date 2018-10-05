#include "hs_interface.h"
#include "../h2codez.h"

HaloScriptInterface *g_halo_script_interface = new HaloScriptInterface;

bool HaloScriptCommon::hs_execute(const char *script, bool ran_from_console)
{
	typedef char (__cdecl *c_hs_execute)(const char *script, int ran_from_console);

	int hs_execute_string_ptr = SwitchAddessByMode(0x005C6440, 0x004E35F0, 0x004D5790);
	CHECK_FUNCTION_SUPPORT(hs_execute_string_ptr);

	c_hs_execute c_hs_execute_impl = reinterpret_cast<c_hs_execute>(hs_execute_string_ptr);
	return c_hs_execute_impl(script, ran_from_console);
}

void **HaloScriptCommon::epilog(void *DatumIndex, int return_data)
{
	typedef void **(__cdecl *hs_epilog)(void *a1, int return_data);
	hs_epilog hs_epilog_impl = reinterpret_cast<hs_epilog>(SwitchAddessByMode(0, 0x52CC70, 0));
	CHECK_FUNCTION_SUPPORT(hs_epilog_impl);

	return hs_epilog_impl(DatumIndex, return_data);
}

void *HaloScriptCommon::prolog(__int16 command_id, void *DatumIndex, char user_cmd)
{
	typedef void *(__cdecl *hs_prolog)(__int16 a1, void *a2, char a3);
	hs_prolog hs_prolog_impl = reinterpret_cast<hs_prolog>(SwitchAddessByMode(0, 0x52D3E0,0 ));
	CHECK_FUNCTION_SUPPORT(hs_prolog_impl);

	return hs_prolog_impl(command_id, DatumIndex, user_cmd);
}

char __cdecl HaloScriptCommon::hs_default_func_check(__int16 opcode, void *DatumIndex)
{
	int hs_default_check = SwitchAddessByMode(0x65F090, 0x581EB0, 0x56E910);
	CHECK_FUNCTION_SUPPORT(hs_default_check);

	func_check hs_default_check_impl = reinterpret_cast<func_check>(hs_default_check);
	return hs_default_check_impl(opcode, DatumIndex);
}

std::string HaloScriptCommon::get_value_as_string(const void *var_ptr, hs_type type)
{
	const static bool false_hack = false;
	std::string value_as_string;
	if (!var_ptr)
		var_ptr = &false_hack;

	switch (type) {
	case hs_type::boolean:
		value_as_string = (*static_cast<const bool*>(var_ptr)) ? "True" : "False";
		return value_as_string;
	case hs_type::hs_long:
		value_as_string = std::to_string(*static_cast<const long*>(var_ptr));
		return value_as_string;
	case hs_type::nothing:
		return "void";;
	case hs_type::real:
		value_as_string = std::to_string(*static_cast<const float*>(var_ptr));
		return value_as_string;
	case hs_type::hs_short:
		value_as_string = std::to_string(*static_cast<const short*>(var_ptr));
		return value_as_string;
	default:
		throw std::runtime_error("Converting HS data of type '" + hs_type_string[type] + "' to string is not implemented.\n");
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

void **__cdecl custom_func_wrapper(int opcode, void *DatumIndex, char user_cmd)
{
	hs_opcode id = static_cast<hs_opcode>(opcode);
	auto custom_func = g_halo_script_interface->get_custom_func(id);
	void *args = HaloScriptCommon::prolog(static_cast<short>(opcode), DatumIndex, user_cmd);
	int return_data = 0;
	if (LOG_CHECK(custom_func) && LOG_CHECK(args))
	{
		return_data = custom_func(args);
	}
	return HaloScriptCommon::epilog(DatumIndex, return_data);
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
		hs_default_func_check,
		custom_func_wrapper,
		cmd_string(custom_cmd.description),
		cmd_string(custom_cmd.custom_usage),
		custom_cmd.args.size(),
		custom_cmd.args.data()
	);
	RegisterCommand(id, command);
}
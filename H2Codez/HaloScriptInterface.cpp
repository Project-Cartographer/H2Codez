#include "HaloScriptInterface.h"
#include "h2codez.h"
#include "HaloScript.h"

HaloScriptInterface *g_halo_script_interface = new HaloScriptInterface;

bool HaloScriptCommon::hs_execute(char *script, bool ran_from_console)
{
	typedef char (__cdecl *c_hs_execute)(char *script, int ran_from_console);

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

void **HaloScriptCommon::prolog(__int16 command_id, void *DatumIndex, char user_cmd)
{
	typedef void **(__cdecl *hs_prolog)(__int16 a1, void *a2, char a3);
	hs_prolog hs_prolog_impl = reinterpret_cast<hs_prolog>(SwitchAddessByMode(0, 0x52CC70,0 ));
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

bool false_hack = false;
std::string HaloScriptCommon::get_value_as_string(void *var_ptr, hs_type type)
{
	std::string value_as_string;
	if (!var_ptr)
		var_ptr = &false_hack;

	switch (type) {
	case hs_type::boolean:
		value_as_string = (*static_cast<bool*>(var_ptr)) ? "True" : "False";
		return value_as_string;
	case hs_type::hs_long:
		value_as_string = std::to_string(*static_cast<long*>(var_ptr));
		return value_as_string;
	case hs_type::nothing:
		return "void";;
	case hs_type::real:
		value_as_string = std::to_string(*static_cast<float*>(var_ptr));
		return value_as_string;
	case hs_type::hs_short:
		value_as_string = std::to_string(*static_cast<short*>(var_ptr));
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
		if (old_table_offset == 905) // extra command in old table
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
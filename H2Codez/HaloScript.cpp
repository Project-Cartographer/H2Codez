#include "stdafx.h"
#include "HaloScript.h"

void **HaloScript::epilog(void *a1, int return_data)
{
	typedef void **(__cdecl *hs_epilog)(void *a1, int return_data);
	hs_epilog hs_epilog_impl = reinterpret_cast<hs_epilog>(0x52CC70);

	return hs_epilog_impl(a1, return_data);
}

void **HaloScript::prolog(__int16 command_id, int a2, char a3)
{
	typedef void **(__cdecl *hs_prolog)(__int16 a1, int a2, char a3);
	hs_prolog hs_prolog_impl = reinterpret_cast<hs_prolog>(0x52CC70);

	return hs_prolog_impl(command_id, a2, a3);
}

// this is a massive hack but whatever
bool false_hack = false;
std::string HaloScript::get_value_as_string(void *var_ptr, hs_type type)
{
	std::string value_as_string;
	if (!var_ptr)
		var_ptr = &false_hack;

	switch (type) {
	case hs_type::boolean:
		value_as_string = *reinterpret_cast<bool*>(var_ptr) ? "True" : "False";
		return value_as_string;
	case hs_type::funtion_name:
		value_as_string = "FIXME: read function name";
		return value_as_string;
	case hs_type::long_int:
		value_as_string = std::to_string(*reinterpret_cast<long*>(var_ptr));
		return value_as_string;
	case hs_type::nothing:
		value_as_string = "<void>";
		return value_as_string;
	case hs_type::passthrough:
		value_as_string = "FIXME: read passthrough";
		return value_as_string;
	case hs_type::real:
		value_as_string = std::to_string(*reinterpret_cast<float*>(var_ptr));
		return value_as_string;
	case hs_type::script:
		value_as_string = "FIXME: read script";
		return value_as_string;
	case hs_type::short_int:
		value_as_string = std::to_string(*reinterpret_cast<short*>(var_ptr));
		return value_as_string;
	case hs_type::special_form:
		value_as_string = "FIXME: read special_form";
		return value_as_string;
	}
	return "Unknown Script Type!!";
}

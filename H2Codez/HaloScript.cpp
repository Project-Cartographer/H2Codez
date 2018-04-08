#include "stdafx.h"
#include "HaloScript.h"

using namespace HaloScriptCommon;

// this is a massive hack but whatever
bool false_hack = false;
std::string HaloScript::get_value_as_string(void *var_ptr, hs_type type)
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

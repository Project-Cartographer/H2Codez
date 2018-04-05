#include "HaloScriptCommon.h"
#include "h2codez.h"

bool HaloScriptCommon::hs_execute(char *script, bool ran_from_console)
{
	typedef char (__cdecl *c_hs_execute)(char *script, int ran_from_console);

	int hs_execute_string_ptr = SwitchAddessByMode(0x005C6440, 0x004E35F0, 0x004D5790);
	CHECK_FUNCTION_SUPPORT(hs_execute_string_ptr);

	c_hs_execute c_hs_execute_impl = reinterpret_cast<c_hs_execute>(hs_execute_string_ptr);
	return c_hs_execute_impl(script, ran_from_console);
}

std::string search_for;
bool matching_string(std::pair<const HaloScriptCommon::ai_behaviour, std::string> i) {
	return i.second == search_for;
}

HaloScriptCommon::ai_behaviour HaloScriptCommon::string_to_ai_behaviour(const std::string &contents)
{
	search_for = contents;
	auto i = std::find_if(ai_behaviour_string.begin(), ai_behaviour_string.end(), matching_string);
	if (i == ai_behaviour_string.end()) {
		return ai_behaviour::invalid;
	}
	else {
		return i->first;
	}
}
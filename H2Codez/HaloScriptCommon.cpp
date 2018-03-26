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
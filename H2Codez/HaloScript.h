#pragma once
#include "H2ToolsCommon.h"

class HaloScript
{
public:
	/* Used to return data to the scripting engine, marks the end of a script command*/
	static void **epilog(void *a1, int return_data);

	/* returns the arguments passed to the command */
	static void **prolog(__int16 command_id, int a2, char a3);

	/* Not finnished, didn't even add all the function types*/
	static std::string get_value_as_string(void *var_ptr, hs_type type);
};


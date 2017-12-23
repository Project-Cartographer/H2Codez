#pragma once
#include <unordered_map>

class H2CommonPatches {
public:
	static void Init();
	static bool newInstance();
	static std::string get_temp_name(std::string name_suffix = "");
	static void generate_script_doc(const char *filename = nullptr);
};

typedef char(__cdecl *func_check)(__int16 a1, unsigned __int16 a2);
typedef void **(__cdecl *func_impl)(int a1, void *a2, char a3);

enum class hs_type {
	unparsed,
	special_form,
	funtion_name,
	passthrough,
	nothing,
	boolean,
	real,
	short_int,
	long_int,
	string,
	script
};

static std::unordered_map <const hs_type, std::string> hs_type_string
{
	{ hs_type::unparsed,     "unparsed" },
	{ hs_type::funtion_name, "function name" },
	{ hs_type::long_int,     "long" },
	{ hs_type::nothing,      "void" },
	{ hs_type::passthrough,  "passthrough" },
	{ hs_type::real,         "real" },
	{ hs_type::script,       "script" },
	{ hs_type::short_int,    "short" },
	{ hs_type::special_form, "special form" },
	{ hs_type::boolean,      "boolean" }
};

struct hs_command
{
	hs_type return_type;
	char *name;
	DWORD unk1 = 0;
	func_check check_command_args;
	func_impl command_impl;
	char *desc = NULL;
	char *usage = NULL;
	DWORD arg_count = 0;
	DWORD arg_mess_ = 0;

	hs_command(char *cmd_name, hs_type ret_type, func_check arg_check, func_impl impl)
	{
		name = cmd_name;
		return_type = ret_type;
		check_command_args = arg_check;
		command_impl = impl;
	}

};
static_assert(sizeof(hs_command) == 0x24, "Bad struct size");

struct hs_global_variable {
	char *name;
	hs_type type;
	void *variable_ptr;
};
static_assert(sizeof(hs_global_variable) == 0xC, "Bad struct size");
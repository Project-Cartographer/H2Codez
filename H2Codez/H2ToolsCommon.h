#pragma once
#include <unordered_map>

class H2CommonPatches {
public:
	static void Init();
	static bool newInstance();
	static std::string get_temp_name(std::string name_suffix = "");
	static void print_help_to_doc();
};

typedef char(__cdecl *func_check)(__int16 a1, unsigned __int16 a2);
typedef int(__cdecl *func_impl)(int _18, int a1, char a3);

struct hs_command
{
	DWORD unk;
	char *name;
	DWORD unk1;
	func_check check_command_args;
	func_impl command_impl;
	char *desc;
	char *usage;
	DWORD arg_count;
};
static_assert(sizeof(hs_command) == 0x20, "Bad struct size");

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
	{ hs_type::unparsed,     "unparsed"      },
	{ hs_type::funtion_name, "function name" },
	{ hs_type::long_int,     "long"          },
	{ hs_type::nothing,      "void"          },
	{ hs_type::passthrough,  "passthrough"   },
	{ hs_type::real,         "real"          },
	{ hs_type::script,       "script"        },
	{ hs_type::short_int,    "short"         },
	{ hs_type::special_form, "special form"  },
	{ hs_type::boolean,      "boolean"       }
};

struct hs_global_variable {
	char *name;
	hs_type type;
	void *variable_ptr;
};

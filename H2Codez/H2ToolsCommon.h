#pragma once
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

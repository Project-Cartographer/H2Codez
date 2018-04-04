#pragma once
#include "HaloScriptCommon.h"

struct hs_convert_data_store
{
	DWORD field_0;
	WORD target_hs_type;
	WORD field_2;
	DWORD field_3;
	DWORD string_value_offset;
	DWORD output;
};

class H2CommonPatches {
public:
	static void Init();
	static bool newInstance();
	static std::string get_temp_name(std::string name_suffix = "");
	static void generate_script_doc(const char *filename = nullptr);
	static void copy_to_clipboard(std::string text, HWND owner = NULL);
	static bool read_clipboard(std::string &contents, HWND owner = NULL);
};


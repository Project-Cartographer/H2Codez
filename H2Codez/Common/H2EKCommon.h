#pragma once
#include "..\HaloScript\hs_interface.h"

struct hs_convert_data_store
{
	WORD field_0;
	WORD hs_constant_type;
	WORD target_hs_type;
	WORD field_6;
	DWORD field_8;
	DWORD string_value_offset;
	DWORD output;
};
CHECK_STRUCT_SIZE(hs_convert_data_store, 0x14);

namespace H2CommonPatches {
	void Init();
	bool newInstance();
	std::string get_temp_name(const std::string &name_suffix = "");
	void generate_script_doc(const char *filename = nullptr);
	bool copy_to_clipboard(const std::string &text, HWND owner);
	bool read_clipboard(std::string &contents, HWND owner = NULL);
	std::wstring GetExeDirectory();
};


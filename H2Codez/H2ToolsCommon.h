#pragma once
#include "HaloScriptCommon.h"

class H2CommonPatches {
public:
	static void Init();
	static bool newInstance();
	static std::string get_temp_name(std::string name_suffix = "");
	static void generate_script_doc(const char *filename = nullptr);
	static void copy_to_clipboard(std::string text, HWND owner = NULL);
	static bool read_clipboard(std::string &contents, HWND owner = NULL);
};


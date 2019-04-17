#pragma once
#include "..\HaloScript\hs_interface.h"

enum wdp_type : signed int
{
	_tool = 0,
	_sapien = 1,
	_game = 2
};

inline int *get_object_at_data_array_index(void *array, unsigned __int16 id)
{
	int _array = reinterpret_cast<int>(array);
	int data = (*(DWORD *)(_array + 68)) + ((*(DWORD *)(_array + 36)) * id);
	return reinterpret_cast<int*>(data);
}

namespace H2CommonPatches {
	void Init();
	std::string get_temp_name(const std::string &name_suffix = "");
	void generate_script_doc(const char *filename = nullptr);
};


#pragma once
#include "HaloScript\hs_interface.h"
#include "util\numerical.h"

enum wdp_type : signed int
{
	_tool = 0,
	_sapien = 1,
	_game = 2
};

namespace H2CommonPatches {
	void Init();

	/* Returns a filename that can be used for a temp file */
	std::string get_temp_name(const std::string &name_suffix = "");

	/* 
		Generates the documentation for HaloScript, saves it to filename then opens it
	*/
	void generate_script_doc(const char *filename = nullptr);

	void dump_loaded_tags();
	void dump_loaded_tags(std::wstring folder);

	std::string get_h2ek_documents_dir();
};

static constexpr int max_bitmap_size = 0x2000;
static constexpr int max_lightmap_size = max_bitmap_size / 2;

static_assert(numerical::is_power_of_two(max_bitmap_size), "bitmap max size is not power of two");
static_assert(numerical::is_power_of_two(max_lightmap_size), "lightmap max size is not power of two");


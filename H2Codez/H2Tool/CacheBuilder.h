#pragma once
#include "..\Common\BlamBaseTypes.h"
#include "util\crc32.h"
#include <string>

namespace CacheBuilder
{

	enum shared_type
	{
		mainmenu,
		shared,
		singleplayer_shared
	};
	struct cache_header
	{
		blam_tag magic;
		// Should be 8
		uint32_t engine_gen;

		uint32_t file_size;
		int field_C;

		uint32_t offset_to_index;
		uint32_t index_stream_size;
		uint32_t tag_buffer_size;
		uint32_t total_stream_size;
		uint32_t virtual_base_address;

		struct _tag_dependency_graph {
			uint32_t offset;
			uint32_t size;
		}tag_dependency_graph;
		
		long_string source_file;
		char version[32];
		enum scnr_type : int
		{
			SinglePlayer = 0,
			Multiplayer = 1,
			MainMenu = 2,
			MultiplayerShared = 3,
			SinglePlayerShared = 4
		};
		scnr_type type;
		crc32::result resource_crc;
		int shared_type;
		char field_158;
		char tracked__maybe;
		char field_15A;
		char field_15B;
		int field_15C;
		int field_160;
		int field_164;
		int field_168;

		struct _debug_string_id {
			uint32_t block_offset;
			uint32_t count;
			uint32_t buffer_size;
			uint32_t indices_offset;
			uint32_t buffer_offset;
		}string_ids;

		char dependency[4];
		FILETIME build_date;
		FILETIME shared_resource_build_date[3];

		char name[32];
		enum _lang
		{
			english,
			japanese,
			german,
			french,
			spanish,
			italian,
			korean,
			chinese,
			portuguese
		}language;
		char tag_name[256];
		int minor_version;
		struct _debug_tag_names {
			uint32_t count;
			uint32_t buffer_offset;
			uint32_t buffer_size;
			uint32_t indices_offset;
		}tag_names;

		struct _language_pack {
			uint32_t offset;
			uint32_t size;
		}language_pack;

		int SecondarySoundGestaltDatumIndex;

		struct _fast_geometry_load_region {
			uint32_t cache_block_offset;
			uint32_t cache_block_size;
		}fast_geometry_load_region;

		int Checksum;
		int MoppCodesChecksum;
		BYTE field_2F8[1284];
		blam_tag foot;
	};
	CHECK_STRUCT_SIZE(cache_header, 0x800);

	struct cache_builder_state
	{
		bool building;
		BYTE pad[1];
		wchar_t map_name[256 + 1];
		crc32::result data_crc;
		HANDLE temp_cache;
		size_t cache_size;
		bool display_precache_progress;
		bool log_precache_progress;
		BYTE padding[6];
		wchar_t uncompressed_cache_location[260];
	};
	CHECK_STRUCT_SIZE(crc32::result, 4);
	CHECK_STRUCT_SIZE(cache_builder_state, 0xC48E88 - 0xC48A68);

	cache_builder_state *get_globals();
	size_t get_size();

	inline const wchar_t *get_temp_cache_path()
	{
		return L"maps\\temporary uncompressed cache file.bin";
	}

	bool start_package(const std::wstring &name);

	/*
		writer functions
	*/

	// appends data without aligning it
	bool write_unaligned(LPCVOID data, size_t size, size_t *start = nullptr);
	// appends data
	bool write(LPCVOID data, size_t size, size_t *start = nullptr);
	// appends data with resource crc update
	bool write_resource(LPCVOID data, size_t size, size_t *data_start);
}

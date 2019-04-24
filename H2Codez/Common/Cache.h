#pragma once
#include "util\crc32.h"
#include "..\Common\BlamBaseTypes.h"

namespace cache
{
	enum shared_type
	{
		mainmenu,
		shared,
		singleplayer_shared
	};

	// header used for halo map or "cache" files
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

	uint32_t calculate_xor_checksum(HANDLE cache_file);
}

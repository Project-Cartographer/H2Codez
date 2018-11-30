#pragma once
#include "util\crc32.h"
#include "Common\Cache.h"
#include <string>

namespace CacheBuilder
{
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
	bool end_package(cache::cache_header *header);

	bool write_header(const cache::cache_header *header);

	inline uint32_t calculate_xor_checksum()
	{
		auto builder_state = get_globals();
		assert(builder_state->building && builder_state->temp_cache != INVALID_HANDLE_VALUE);

		auto xor_checksum = cache::calculate_xor_checksum(builder_state->temp_cache);
		std::cout << "Realtime data / xor checksum is 0x" << std::hex << xor_checksum << std::endl;
		return xor_checksum;
	}

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

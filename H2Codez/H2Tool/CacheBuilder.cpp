#include "H2Tool_Commands.h"
#include "util\Patches.h"
#include "util\crc32.h"
#include "util\Logs.h"
#include "Common\H2EKCommon.h"

using namespace CacheBuilder;

cache_builder_state *CacheBuilder::get_globals()
{
	if (game.process_type == H2EK::H2Tool) {
		return H2ToolPatches::get_build_cache_file_globals();
	}
	else {
		static cache_builder_state state;
		return &state;
	}
}

size_t CacheBuilder::get_size()
{
	auto globals = get_globals();
	return LOG_CHECK(GetFileSize(globals->temp_cache, 0));
}

bool CacheBuilder::start_package(const std::wstring &name)
{
	auto builder_state = get_globals();
	if (!builder_state->building)
	{
		LOG_FUNC("Already building cache!");
		return false;
	}
	HANDLE cache_file = CreateFileW(get_temp_cache_path(), GENERIC_WRITE | GENERIC_READ, is_debug_build() ? FILE_SHARE_READ : 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!LOG_CHECK(cache_file != INVALID_HANDLE_VALUE))
	{
		LOG_FUNC("Failed to create new cache file!");
		return false;
	}
	builder_state->building = true;
	builder_state->cache_size = 0;
	builder_state->data_crc = crc32::result();
	builder_state->temp_cache = cache_file;
	wcsncpy(builder_state->map_name, name.c_str(), 0x100u);

	cache_header empty_header;
	memset(&empty_header, 0xFF, sizeof(empty_header));
	return write_unaligned(&empty_header, sizeof(empty_header));
}

bool CacheBuilder::write_unaligned(LPCVOID data, size_t size, size_t *start)
{
	auto cache_state = get_globals();
	if (!LOG_CHECK(cache_state->building && cache_state->temp_cache != INVALID_HANDLE_VALUE))
	{
		LOG_FUNC("Builder in invalid state!");
		return false;
	}
	if (!LOG_CHECK(data))
		return false;
	if (size == 0)
		return true;
	DWORD bytes_written;
	if (LOG_CHECK(WriteFile(cache_state->temp_cache, data, size, &bytes_written, NULL))
		&& LOG_CHECK(bytes_written == size))
	{
		if (start)
			*start = cache_state->cache_size;
		cache_state->cache_size += size;
		return true;
	}
	return false;
}

bool CacheBuilder::write(LPCVOID data, size_t size, size_t *start)
{
	const static size_t padding_size = conf.getNumber<size_t>("cache_file_alignment", 0x200);
	const static char *padding = new char[padding_size]();

	auto get_next_aligned = [=]() -> size_t
	{
		if (size % padding_size == 0)
			return size;
		return (size / padding_size + 1) * padding_size;
	};

	if (is_debug_build() && size == 0)
	{
		LOG_FUNC("Rezero length data written to cache");
	}
	return LOG_CHECK(CacheBuilder::write_unaligned(data, size, start)) &&
		(padding_size == 0 || LOG_CHECK(CacheBuilder::write_unaligned(padding, get_next_aligned() - size)));
}


bool CacheBuilder::write_resource(LPCVOID data, size_t size, size_t *data_start)
{
	auto cache_state = get_globals();
	crc32::calculate(cache_state->data_crc, data, size);
	return CacheBuilder::write(data, size, data_start);
}

char __cdecl cache_writer_write_data(BYTE *data, size_t size, size_t *data_start)
{
	return CacheBuilder::write_resource(data, size, data_start);
}
void H2ToolPatches::patch_cache_writter()
{
	WriteJmp(0x607BB0, cache_writer_write_data);
}

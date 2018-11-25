#include "H2Tool_Commands.h"
#include "util\Patches.h"
#include "util\crc32.h"

CHECK_STRUCT_SIZE(crc32::result, 4);

bool write_to_cache(LPCVOID data, size_t size, size_t *start = nullptr)
{
	if (!LOG_CHECK(data))
		return false;
	if (size == 0)
		return true;
	DWORD bytes_written;
	auto cache_state = H2ToolPatches::get_build_cache_file_globals();
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

bool write_to_cache_with_padding(LPCVOID data, size_t size, size_t *start = nullptr)
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
		pLog.WriteLog("Rezero length data written to cache");
	}
	return LOG_CHECK(write_to_cache(data, size, start)) &&
		(padding_size == 0 || LOG_CHECK(write_to_cache(padding, get_next_aligned() - size)));
}


char __cdecl cache_writer_write_data(BYTE *data, size_t size, size_t *data_start)
{
	auto cache_state = H2ToolPatches::get_build_cache_file_globals();
	crc32::calculate(cache_state->data_crc, data, size);
	return write_to_cache_with_padding(data, size, data_start);
}

void H2ToolPatches::patch_cache_writter()
{
	WriteJmp(0x607BB0, cache_writer_write_data);
}

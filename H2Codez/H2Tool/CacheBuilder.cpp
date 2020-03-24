#include "H2Tool.h"
#include "util\Patches.h"
#include "util\crc32.h"
#include "util\Logs.h"
#include "Common\H2EKCommon.h"
#include "Common\TagInterface.h"
#include "Tags\ScenarioTag.h"
#include <timeapi.h>


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

	cache::cache_header empty_header;
	memset(&empty_header, 0xFF, sizeof(empty_header));
	return write_unaligned(&empty_header, sizeof(empty_header));
}

bool CacheBuilder::end_package(cache::cache_header *header)
{
	auto builder_state = get_globals();
	auto size = get_size();
	std::cout << "cache size: " << size / (1024 * 1024) << "mb" << endl;
	header->file_size = size;
	header->resource_crc = builder_state->data_crc;
	header->Checksum = calculate_xor_checksum();

	if (!write_header(header))
	{
		printf("Failed to write cache header");
		return false;
	}

	// close and copy
	if (!LOG_CHECK(CloseHandle(builder_state->temp_cache)))
	{
		printf("Failed to close cache");
		return false;
	}
	std::wstring new_name = L"maps//" + std::wstring(builder_state->map_name) + L".map";
	return MoveFileExW(get_temp_cache_path(), new_name.c_str(), MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
}

bool CacheBuilder::write_header(const cache::cache_header *header)
{
	auto cache_state = get_globals();
	LOG_CHECK(SetFilePointer(cache_state->temp_cache, 0, NULL, FILE_BEGIN) != INVALID_FILE_SIZE);
	return write_unaligned(header, sizeof(cache::cache_header));
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
		LOG_FUNC("zero length data written to cache");
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

char __stdcall build_cache_file_write_header_and_compress(cache::cache_header *header)
{
	if (CacheBuilder::end_package(header))
	{
		printf("Done\n");
		return 1;
	} else {
		printf("FAILED!\n");
		return 0;
	}
}

 char ASM_FUNC build_cache_file_write_header_and_compress_wrapper()
{
	 __asm {
		 push esi
		 call build_cache_file_write_header_and_compress
		 retn
	 }
}

 // returns scenario pointer
 static scnr_tag *get_global_scenario()
 {
	 return *reinterpret_cast<scnr_tag **>(0x00AA00E4);
 }


bool build_cache_file_cull_tags()
 {
	 tags::block_delete_all(&get_global_scenario()->sourceFiles);
	 tags::block_delete_all(&get_global_scenario()->comments);
	 tags::block_delete_all(&get_global_scenario()->decorators);
	 return 1;
}

static constexpr double internal_in_seconds(int milliseconds_start, int milliseconds_end)
{
	return (milliseconds_end - milliseconds_start) / 1000.0;
}

static void package_map_log(const char *format, ...)
{
	va_list ArgList;
	va_start(ArgList, format);
	vprintf_s(format, ArgList); // todo: log to filo
}

#define time_code(name, ...) \
	operation_time = timeGetTime(); \
	(__VA_ARGS__); \
	package_map_log(name " %f seconds %d bytes\n", internal_in_seconds(operation_time, timeGetTime()), CacheBuilder::get_size());
	

bool build_cache_file(const char *scenario, int minor_version = 0, bool something = true)
{
	int operation_time;
	int start_time = timeGetTime();
	time_code("build_cache_file_cull_tags", build_cache_file_cull_tags());
	return true;
}

void H2ToolPatches::patch_cache_writter()
{
	// disable compression buffer allocation as it remains unused
	NopFill(0x587F79, 5); // debug_malloc call for compression buffer
	NopFill(0x589454, 5); // debug_free call for compression buffer

	if (conf.getBoolean("enable_cache_writer_patches", false)) {
		WriteJmp(0x607BB0, cache_writer_write_data);
		PatchCall(0x589279, build_cache_file_write_header_and_compress_wrapper);
	}
}

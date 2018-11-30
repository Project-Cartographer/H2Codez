#include "Cache.h"

#define file_chunk_size 1024 * 32
uint32_t cache::calculate_xor_checksum(HANDLE cache_file)
{
	uint32_t checksum_out = 0;

	LOG_CHECK(SetFilePointer(cache_file, sizeof(cache_header), NULL, FILE_BEGIN) != INVALID_FILE_SIZE);

	DWORD bytes_read = 0;
	BYTE file_chunk[file_chunk_size];
	while (LOG_CHECK(ReadFile(cache_file, file_chunk, file_chunk_size,
		&bytes_read, NULL)))
	{
		if (bytes_read == 0)
			break;

		for (auto byte : file_chunk)
			checksum_out ^= byte;
	}
	return checksum_out;
}

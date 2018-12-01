#include "Cache.h"

#define file_chunk_size 1024 * 1024

inline uint32_t checksum_chunk(BYTE *data, size_t size)
{
	uint32_t checksum_out;
	if (game.process_type == H2EK::H2Tool)
	{
		typedef int __cdecl calc_real_time_checksum(BYTE *data, unsigned int size);
		auto calc_real_time_checksum_impl = reinterpret_cast<calc_real_time_checksum*>(0x57EB30);
		checksum_out = calc_real_time_checksum_impl(data, size);
	} else { // something is wrong with this
		for (size_t i = 0; i < size; i++)
			checksum_out ^= data[i];
	}
	return checksum_out;
}

uint32_t cache::calculate_xor_checksum(HANDLE cache_file)
{
	uint32_t checksum_out = 0;

	LOG_CHECK(SetFilePointer(cache_file, sizeof(cache_header), NULL, FILE_BEGIN) != INVALID_FILE_SIZE);

	DWORD bytes_read = 0;
	BYTE *file_chunk = new BYTE[file_chunk_size];
	while (LOG_CHECK(ReadFile(cache_file, file_chunk, file_chunk_size,
		&bytes_read, NULL)))
	{
		if (bytes_read == 0)
			break;

		checksum_out ^= checksum_chunk(file_chunk, bytes_read);
	}
	delete[] file_chunk;
	return checksum_out;
}

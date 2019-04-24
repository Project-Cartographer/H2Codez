#pragma once
#include "Common\MemoryAllocator.h"

struct s_data_array
{
	char name[32];
	int max_count;
	int datum_size;
	char alignment;
	char is_valid;
	__int16 flags;
	int magic;
	c_allocation_base *allocator;
	int next_index;
	int bit_index_length;
	int size;
	int next_datum;
	char *data;
	char *active_indices_bit_vector;

	template<typename T = void>
	inline T *datum_get(unsigned short index)
	{
		return reinterpret_cast<T*>(&active_indices_bit_vector[datum_size * index]);
	}
};
static_assert(sizeof(s_data_array) == 0x4C, "bad s_data_array size");

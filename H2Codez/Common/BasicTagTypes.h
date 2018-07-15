#pragma once

#include "BlamBaseTypes.h"

#define NONE -1

struct tag_ref
{
	int tag_type;
	void *tag_pointer;
	int field_8;
	int tag_index;
};

struct tag_block_ref
{
	int size;
	void *data;
	void *definition;
};

template<typename T>
struct tag_block
{
	int size;
	T *data;
	void *defination;

	tag_block_ref* get_ref() {
		return reinterpret_cast<tag_block_ref*>(this);
	}
	tag_block_ref *operator&()
	{
		return get_ref();
	}
};

struct byte_ref
{
	int size;
	int stream_flags;
	int stream_offset;
	void *address;
	int definition;
};
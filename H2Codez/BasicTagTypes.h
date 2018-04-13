#pragma once

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

struct byte_ref
{
	int size;
	int stream_flags;
	int stream_offset;
	void *address;
	int definition;
};
#pragma once
#include "BasicTagTypes.h"

namespace tags
{
	int get_group_tag(datum tag);
	tag_block_ref *get_root_block(datum tag);
	std::string get_name(datum tag);
	bool tag_exists(int group, std::string path);
	bool is_read_only(datum tag);

	void set_reference(tag_ref *reference, int group_tag, cstring name);
	void copy_reference(tag_ref *src, tag_ref *dest);

	template<typename T = void>
	inline T *get_tag(int tag_group, datum index)
	{
		return reinterpret_cast<T*>(get_tag<void>(tag_group, index));
	}

	template<>
	void *get_tag<void>(int tag_group, datum index);

	void unload_tag(datum index);
	bool is_tag_loaded(int tag_group, const std::string name);
	datum new_tag(int group, const std::string name);
	datum load_tag(int group, const std::string name, int flags);
	size_t reload_tag(int group, const std::string name);

	bool rename_tag(datum index, const std::string name);
	bool save_tag(datum index);

	unsigned int add_block_element(tag_block_ref *block);
	bool delete_block_element(tag_block_ref *block, size_t index);
	bool block_delete_all(tag_block_ref *block);
	bool copy_block(tag_block_ref *source_block, tag_block_ref *dest_block);
	bool resize_block(tag_block_ref *block, size_t size);
};

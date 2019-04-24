#pragma once
#include "BasicTagTypes.h"

namespace tags
{
	/* Get tag group of tag */
	int get_group_tag(datum tag);

	/* Get main tag block from tag */
	tag_block_ref *get_root_block(datum tag);

	/* Get tag name */
	std::string get_name(datum tag);

	/* Check if a tag of group exists at path */
	bool tag_exists(int group, std::string path);

	/* Is the tag file read-only */
	bool is_read_only(datum tag);

	void set_reference(tag_ref *reference, int group_tag, cstring name);
	void copy_reference(tag_ref *src, tag_ref *dest);

	/* Return the data for a tag */
	template<typename T = void>
	inline T *get_tag(int tag_group, datum index)
	{
		return reinterpret_cast<T*>(get_tag<void>(tag_group, index));
	}

	template<>
	void *get_tag<void>(int tag_group, datum index);

	/* Unload a tag */
	void unload_tag(datum index);

	/* Checks if tag is currently loaded */
	bool is_tag_loaded(blam_tag tag_group, const std::string name);

	/* Create new tag with group and name */
	datum new_tag(int group, const std::string name);

	/* Loads a tag from file system */
	datum load_tag(int group, const std::string name, int flags);

	/* Reloads tag data from file system if possible */
	size_t reload_tag(blam_tag group, const std::string name);

	/* Change the path the tag will be saved to */
	bool rename_tag(datum index, const std::string name);

	/* Save tag to disk */
	bool save_tag(datum index);

	/* Add a new element, returns index */
	template<typename T = void>
	unsigned int add_block_element(tag_block<T> *block)
	{
		return add_block_element(block->get_ref());
	}
	unsigned int add_block_element(tag_block_ref *block);

	/* Deletes element at index */
	template<typename T = void>
	bool delete_block_element(tag_block<T> *block, size_t index)
	{
		return delete_block_element(block->get_ref(), index);
	}
	bool delete_block_element(tag_block_ref *block, size_t index);

	/* Delete all tag block elements */
	template<typename T = void>
	bool block_delete_all(tag_block<T> *block)
	{
		return block_delete_all(block->get_ref());
	}
	bool block_delete_all(tag_block_ref *block);

	/* copy tag block elements */
	template<typename T = void>
	bool copy_block(tag_block<T> *source_block, tag_block<T> *dest_block)
	{
		return copy_block(source_block->get_ref(), dest_block->get_ref());
	}
	bool copy_block(tag_block_ref *source_block, tag_block_ref *dest_block);

	/* Change number of tag block elements */
	template<typename T = void>
	bool resize_block(tag_block<T> *block, size_t size)
	{
		return resize_block(block->get_ref(), size);
	}
	bool resize_block(tag_block_ref *block, size_t size);
};

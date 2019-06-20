#pragma once
#include "BasicTagTypes.h"
#include "common/data/data_array.h"
#include "TagDefinitions.h"
#include "FiloInterface.h"

namespace tags
{
	inline s_data_array *get_tag_instances()
	{
		return *reinterpret_cast<s_data_array**>(SwitchAddessByMode(0xA801A0, 0xA75488, 0x9B7E20));
	}

	struct s_tag_instance
	{
		__int16 datum_header;
		__int16 flags;
		const char *name;
		int group_tag;
		int parent_group;
		int grandparent_group;
		char field_14;
		char read_only;
		char field_16;
		char field_17;
		char loading_finished__maybe;
		char field_19;
		char sub_4AEC40;
		char field_1B;
		int salt;
		tag_block_ref data;
		int next__maybe;
	};
	CHECK_STRUCT_SIZE(s_tag_instance, 0x30);


	struct s_tag_ilterator
	{
		void *next_tag_instance;
		s_data_array::s_ilterator ilterator;
		blam_tag tag_group;
		s_tag_ilterator(blam_tag tag = blam_tag::null()) :
			ilterator(get_tag_instances()),
			tag_group(tag)
		{
		};

		datum next()
		{
			typedef signed int __cdecl tag_next(s_tag_ilterator *a1);
			auto tag_next_impl = reinterpret_cast<tag_next*>(SwitchAddessByMode(0x52DD30, 0x4AF700, 0x4836D0));
			return tag_next_impl(this);
		}
	};
	CHECK_STRUCT_SIZE(s_tag_ilterator, 0x18);

	/* Get tag_def for tag_group */
	tag_def *get_group_definition(int tag_group);

	/* Get tag group of tag */
	int get_group_tag(datum tag);

	/* Get tag_def for tag_group */
	inline tag_def *get_group_definition(datum tag)
	{
		return get_group_definition(get_group_tag(tag));
	}

	/* Get main tag block from tag */
	tag_block_ref *get_root_block(datum tag);

	/* Get tag name */
	std::string get_name(datum tag);

	/* Check if a tag of group exists at path */
	bool tag_exists(int group, std::string path);

	/* Is the tag file read-only */
	bool is_read_only(datum tag);

	void set_reference(tag_reference *reference, int group_tag, cstring name);
	void copy_reference(tag_reference *src, tag_reference *dest);

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

	/* Get filo for tag */
	bool get_tag_filo(filo *file_ref, int tag_group, LPCSTR tag_path);
	inline bool get_tag_filo(filo *file_ref, datum tag)
	{
		std::string name = get_name(tag);
		int tag_group = get_group_tag(tag);
		return get_tag_filo(file_ref, tag_group, name.c_str());
	}
};

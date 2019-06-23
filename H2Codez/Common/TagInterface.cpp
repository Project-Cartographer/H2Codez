#include "TagInterface.h"
#include "H2EKCommon.h"
#include "util/Patches.h"

inline DWORD OS_switch_by_addr(DWORD guerilla, DWORD tool, DWORD sapien)
{
	return SwitchAddessByMode(tool, sapien, guerilla);
}

namespace tags
{
	bool get_tag_filo(filo *file_ref, int tag_group, LPCSTR tag_path)
	{
		typedef char __cdecl _get_tag_filo(filo *file_ref, int tag_group, LPCSTR tag_path);
		auto _get_tag_filo_impl = reinterpret_cast<_get_tag_filo*>(SwitchAddessByMode(0, 0x4B8A10, 0));
		if (_get_tag_filo_impl)
			return _get_tag_filo_impl(file_ref, tag_group, tag_path);
		return false;
	}

	tag_def *get_group_definition(int tag_group)
	{
		typedef tag_def *__cdecl tag__get_group_definition(int tag_group);
		auto tag__get_group_definition_impl = reinterpret_cast<tag__get_group_definition*>(SwitchAddessByMode(0, 0x4B0030, 0));
		CHECK_FUNCTION_SUPPORT(tag__get_group_definition_impl);
		return tag__get_group_definition_impl(tag_group);
	}

	int get_group_tag(datum tag)
	{
		return get_tag_instances()->datum_get<s_tag_instance>(tag.index)->group_tag;
	}

	tag_block_ref *get_root_block(datum tag)
	{
		return &get_tag_instances()->datum_get<s_tag_instance>(tag.index)->data;
	}

	std::string get_name(datum tag)
	{
		auto name = get_tag_instances()->datum_get<s_tag_instance>(tag.index)->name;
		return name ? name : "";
	}

	bool tag_exists(int group, std::string path)
	{
		filo tag_ref;
		if (get_tag_filo(&tag_ref, group, path.c_str()) && FiloInterface::check_access(&tag_ref))
			return true;
		return false;
	}

	bool is_read_only(datum tag)
	{
		return get_tag_instances()->datum_get<s_tag_instance>(tag.index)->read_only;
	}

	void set_reference(tag_reference *reference, int group_tag, cstring name)
	{
		typedef void __cdecl tag_set_reference(tag_reference *reference, int group_tag, cstring name);
		auto tag_set_reference_impl = reinterpret_cast<tag_set_reference*>(OS_switch_by_addr(0x482C40, 0x52D290, 0x4AEC60));
		tag_set_reference_impl(reference, group_tag, name);
	}

	void copy_reference(tag_reference *src, tag_reference *dest)
	{
		typedef void __cdecl copy_reference(tag_reference *src, tag_reference *dest);
		auto copy_reference_impl = reinterpret_cast<copy_reference*>(OS_switch_by_addr(0x482C80, 0x52D2D0, 0x4AECA0));
		copy_reference_impl(src, dest);
	}

	template<> void *get_tag<void>(blam_tag tag_group, datum index)
	{
		typedef void* __cdecl TAG_GET(datum tag_group, datum tag_datum);
		auto TAG_GET_IMPL = reinterpret_cast<TAG_GET*>(OS_switch_by_addr(0x484A00, 0x52F150, 0x4B0B20));
		return TAG_GET_IMPL(tag_group.as_int(), index);
	}

	bool block_delete_all(tag_block_ref *block)
	{
		typedef char __cdecl TAG_BLOCK_DELETE_ALL(tag_block_ref *a1);
		auto TAG_BLOCK_DELETE_ALL_IMPL = reinterpret_cast<TAG_BLOCK_DELETE_ALL*>(OS_switch_by_addr(0x483380, 0x52D9E0, 0x4AF3B0));
		return TAG_BLOCK_DELETE_ALL_IMPL(block);
	}

	void unload_tag(datum index)
	{
		typedef void __cdecl TAG_UNLOAD(datum index);
		auto TAG_UNLOAD_IMPL = reinterpret_cast<TAG_UNLOAD*>(OS_switch_by_addr(0x484E40, 0x52F7C0, 0x4B1190));
		return TAG_UNLOAD_IMPL(index);
	}

	bool rename_tag(datum index, const std::string name)
	{
		typedef char __cdecl Tag__Rename(datum tag_index, const char *new_name);
		auto Tag__Rename_IMPL = reinterpret_cast<Tag__Rename*>(OS_switch_by_addr(0x484E90, 0x52F840, 0x4B11E0));
		return Tag__Rename_IMPL(index, name.c_str());
	}

	bool is_tag_loaded(blam_tag tag_group, const std::string name)
	{
		typedef int __cdecl TAG_LOADED(int tag_group, const char *name);
		auto TAG_LOADED_IMPL = reinterpret_cast<TAG_LOADED*>(OS_switch_by_addr(0x484FF0, 0x52F9A0, 0x4B1340));
		return TAG_LOADED_IMPL(tag_group.as_int(), name.c_str()) != NONE;
	}

	datum new_tag(int group, const std::string name)
	{
		typedef size_t __cdecl TAG_NEW(int group, const char *path);
		auto TAG_NEW_IMPL = reinterpret_cast<TAG_NEW*>(OS_switch_by_addr(0x486A00, 0x5313F0, 0x4B2E10));
		return TAG_NEW_IMPL(group, name.c_str());
	}

	bool save_tag(datum index)
	{
		if (!index.is_valid())
		{
			LOG_FUNC("Tag is invalid");
			return false;
		}
		typedef char __cdecl TAG_SAVE(int tag_index);
		auto TAG_UNLOAD_IMPL = reinterpret_cast<TAG_SAVE*>(OS_switch_by_addr(0x4883A0, 0x532F40, 0x4B47C0));
		return TAG_UNLOAD_IMPL(index.index);
	}
	datum load_tag(blam_tag group, const std::string name, int flags)
	{
		typedef int __cdecl TAG_LOAD(int tag_group, const char *tag_name, int flags);
		auto TAG_LOAD_IMPL = reinterpret_cast<TAG_LOAD*>(OS_switch_by_addr(0x488D90, 0x533930, 0x4B51B0));
		return TAG_LOAD_IMPL(group.as_int(), name.c_str(), flags);
	}

	unsigned int add_block_element(tag_block_ref *block)
	{
		typedef unsigned int __cdecl TAG_BLOCK_ADD_ELEMENT(tag_block_ref *a1);
		auto TAG_BLOCK_ADD_ELEMENT_IMPL = reinterpret_cast<TAG_BLOCK_ADD_ELEMENT*>(OS_switch_by_addr(0x489100, 0x533CA0, 0x4B5520));
		return TAG_BLOCK_ADD_ELEMENT_IMPL(block);
	}

	size_t reload_tag(blam_tag group, const std::string name)
	{
		typedef signed int __cdecl tag_reload(int tag_group, const char *tag_name);
		auto tag_reload_impl = reinterpret_cast<tag_reload*>(OS_switch_by_addr(0x489670, 0x534210, 0x4B5A90));
		return tag_reload_impl(group.as_int(), name.c_str());
	}

	bool copy_block(tag_block_ref *source_block, tag_block_ref *dest_block)
	{
		typedef char __cdecl TAG_BLOCK_COPY(tag_block_ref *source_block, tag_block_ref *dest_block);
		auto tag_block_copy = reinterpret_cast<TAG_BLOCK_COPY*>(OS_switch_by_addr(0x489CB0, 0x534810, 0x4B60D0));
		return tag_block_copy(source_block, dest_block);
	}

	bool delete_block_element(tag_block_ref *block, size_t index)
	{
		typedef char __cdecl TAG_BLOCK_DELETE_ELEMENT(tag_block_ref *ref, size_t index);
		auto TAG_BLOCK_DELETE_ELEMENT_IMPL = reinterpret_cast<TAG_BLOCK_DELETE_ELEMENT*>(OS_switch_by_addr(0x489DB0, 0x534910, 0x4B61D0));
		return TAG_BLOCK_DELETE_ELEMENT_IMPL(block, index);
	}

	bool resize_block(tag_block_ref *block, size_t size)
	{
		typedef char __cdecl tag_block_resize(tag_block_ref *block, size_t new_size);
		auto tag_block_resize_impl = reinterpret_cast<tag_block_resize*>(OS_switch_by_addr(0x489840, 0x5343E0, 0x4B5C60));
		return tag_block_resize_impl(block, size);
	}

}

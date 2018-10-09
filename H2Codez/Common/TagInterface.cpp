#include "TagInterface.h"
#include "H2EKCommon.h"
#include "../util/Patches.h"
#include "FiloInterface.h"

inline int *get_tag_instance_ptr()
{
	return ReadFromAddress<int*>(SwitchAddessByMode(0xA801A0, 0xA75488, 0));
}

inline DWORD OS_switch_by_addr(DWORD guerilla, DWORD tool, DWORD sapien)
{
	return SwitchAddessByMode(tool, sapien, guerilla);
}

char get_tag_filo(filo *file_ref, int tag_group, LPCSTR tag_path)
{
	typedef char __cdecl _get_tag_filo(filo *file_ref, int tag_group, LPCSTR tag_path);
	auto _get_tag_filo_impl = reinterpret_cast<_get_tag_filo*>(SwitchAddessByMode(0, 0x4B8A10, 0));
	if (_get_tag_filo_impl)
		return _get_tag_filo_impl(file_ref, tag_group, tag_path);
	return false;
}

namespace tags
{
	int get_group_tag(datum tag)
	{
		return get_object_at_data_array_index(get_tag_instance_ptr(), tag.index)[2];
	}

	tag_block_ref *get_root_block(datum tag)
	{
		return (tag_block_ref *)(get_object_at_data_array_index(get_tag_instance_ptr(), tag.index) + 8);
	}

	const char *get_name(datum tag)
	{
		return (const char *)get_object_at_data_array_index(get_tag_instance_ptr(), tag.index)[1];
	}

	bool exists(int group, std::string path)
	{
		filo tag_ref;
		if (get_tag_filo(&tag_ref, group, path.c_str()) && FiloInterface::check_access(&tag_ref))
			return true;
		return false;
	}
}

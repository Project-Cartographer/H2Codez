#pragma once
#include "BasicTagTypes.h"

namespace tags
{
	int get_group_tag(datum tag);
	tag_block_ref *get_root_block(datum tag);
	const char *get_name(datum tag);
	bool exists(int group, std::string path);
};

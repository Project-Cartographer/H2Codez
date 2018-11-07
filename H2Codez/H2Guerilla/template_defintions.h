#pragma once
#include "Common\BlamBaseTypes.h"

struct tag_template_class
{
	void *vtbl;
	size_t unk;
	size_t unk1;
};
CHECK_STRUCT_SIZE(tag_template_class, 12);

typedef tag_template_class* (__cdecl *tempate_constructor)(void);

struct tag_block_template_def
{
	blam_tag type;
	size_t unk;
	tempate_constructor constructor;
};
CHECK_STRUCT_SIZE(tag_block_template_def, 12);

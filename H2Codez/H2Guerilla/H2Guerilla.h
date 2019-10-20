#pragma once
#include "Common\TagDefinitions.h"

struct field_information
{
	void *vtble;
	int field_4;
	tag_field *field;
	void *field_C;
	int parent_tag_info;
	field_information *next;
	int field_18;
};


class H2GuerrilaPatches {
public:
	static void Init();
	static void update_field_display();
};

#pragma once
#pragma pack(1)
#include "Common/BasicTagTypes.h"

struct multiplayer_universal_block
{
	// TagReference("unic")
	tag_ref randomPlayerNames;
	// TagReference("unic")
	tag_ref teamNames;

	struct multiplayer_color_block
	{
		colour_rgb color;
	};
	CHECK_STRUCT_SIZE(multiplayer_color_block, 12);
	tag_block<multiplayer_color_block> teamColors;

	// TagReference("unic")
	tag_ref multiplayerText;
};
CHECK_STRUCT_SIZE(multiplayer_universal_block, 60);

struct multiplayer_globals_block
{
	tag_block<multiplayer_universal_block> universal;

	tag_block_ref runtime;
};
CHECK_STRUCT_SIZE(multiplayer_globals_block, 24);


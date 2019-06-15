#pragma once
#include <string>

enum ai_combat_statuses
{
	certain,
	idle,
	unk_2,
	active,
	uninspected,
	unk_5,
	unk_6,
	unk_7,
	clear_los,
	unk_9
};

const static std::string ai_combat_statuses_names[] = {
	"certain",
	"idle",
	"unk_2",
	"active",
	"uninspected",
	"unk_5",
	"unk_6",
	"unk_7",
	"clear_los",
	"unk_9"
};

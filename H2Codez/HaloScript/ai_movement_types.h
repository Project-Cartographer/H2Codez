#pragma once

enum ai_movment_types
{
	patrol,
	sleep,
	combat,
	flee,
	panic,
	t_pose,
	flee_b
};

const static std::string ai_movment_types_names[] = {
	"patrol",
	"sleep",
	"combat",
	"flee",
	"panic",
	"t_pose",
	"flee_b"
};

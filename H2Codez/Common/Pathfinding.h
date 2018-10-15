#pragma once
#include "../Tags/ScenarioStructureBSP.h"
#include <set>

namespace pathfinding
{
	struct render_info
	{
		struct line_set
		{
			std::set<size_t> lines;
		};
		std::map<size_t, line_set> sector_lines;
	};
	bool generate(datum sbsp_tag);
	render_info get_render_info(scenario_structure_bsp_block *sbsp);
}
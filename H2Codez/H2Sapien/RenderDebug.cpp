#include "RenderDebug.h"
#include "..\Common\H2EKCommon.h"
#include "..\util\Patches.h"
#include "..\Tags\ScenarioStructureBSP.h"
#include "..\stdafx.h"

scenario_structure_bsp_block *get_sbsp()
{
	return *reinterpret_cast<scenario_structure_bsp_block **>(0xA9CA74);
}

void draw_debug_line(real_point3d *v0, real_point3d *v1, 
	const colour_rgba *start_colour = nullptr, 
	const colour_rgba *end_colour = nullptr)
{
	const static colour_rgba black;

	if (!start_colour)
	{
		start_colour = &black;
	}

	typedef void __cdecl draw_debug_line(real_point3d *v0, real_point3d *v1, const colour_rgba *start_colour, const colour_rgba *end_colour);
	auto draw_debug_line_impl = reinterpret_cast<draw_debug_line*>(0x715AF0);
	draw_debug_line_impl(v0, v1, start_colour, end_colour);
}

const colour_rgba pathfinding_debug_colour( 1.0f, 0.0f, 1.0f, 1.0f );

void __cdecl render_debug_info_game_in_progress()
{
	if (conf.getBoolean("render_pathfinding_debug", false))
	{
		auto bsp = get_sbsp();
		if (bsp)
		{
			auto pathfinding = bsp->pathfindingData[0];
			if (pathfinding)
			{
				for (const auto sector : pathfinding->sectors)
				{
					auto current_link = pathfinding->links[sector.firstLinkdoNotSetManually];
					while (current_link)
					{
						auto vertex1 = pathfinding->vertices[current_link->vertex1];
						auto vertex2 = pathfinding->vertices[current_link->vertex2];

						if (vertex1 && vertex1)
							draw_debug_line(&vertex1->point, &vertex2->point, &pathfinding_debug_colour);

						if (current_link->forwardLink == sector.firstLinkdoNotSetManually)
							break; // exit if we made a full loop
						if (current_link->linkFlags & current_link->SectorLinkMagicHangingLink)
							break; // just a guess the game might actually handle this differently
						current_link = pathfinding->links[current_link->forwardLink];
					}
				}
			}
		}
	}
}

void H2SapienPatches::render_debug_info_init()
{
	PatchCall(0x6D31F0, render_debug_info_game_in_progress);
}
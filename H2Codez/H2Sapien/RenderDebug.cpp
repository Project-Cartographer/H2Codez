#include "RenderDebug.h"
#include <d3d9.h>
#include "..\Common\H2EKCommon.h"
#include "..\util\Patches.h"
#include "..\Tags\ScenarioStructureBSP.h"
#include "..\stdafx.h"
#include "..\util\string_util.h"

scenario_structure_bsp_block *get_sbsp()
{
	return *reinterpret_cast<scenario_structure_bsp_block **>(0xA9CA74);
}

IDirect3DDevice9Ex *get_global_d3d_device()
{
	return *reinterpret_cast<IDirect3DDevice9Ex **>(0xFE6B34);
}

struct s_debug_vertex
{
	real_point3d point;
	D3DCOLOR colour;
};
CHECK_STRUCT_SIZE(s_debug_vertex, 0x10);

inline D3DCOLOR halo_colour_to_d3d_colour(const colour_rgba *colour)
{
	auto halo_to_hex = [](float num) -> int { return int(num * 255); };

	return D3DCOLOR_ARGB(halo_to_hex(colour->alpha), halo_to_hex(colour->red), halo_to_hex(colour->green), halo_to_hex(colour->blue));
}

void draw_debug_line(real_point3d *v0, real_point3d *v1, 
	const colour_rgba *start_colour = nullptr, 
	const colour_rgba *end_colour = nullptr)
{
	const static colour_rgba white;
	if (!start_colour)
	{
		start_colour = &white;
	}

	if (!end_colour)
	{
		end_colour = start_colour;
	}

	s_debug_vertex line_info[2];
	line_info[0].point = *v0;
	line_info[0].colour = halo_colour_to_d3d_colour(start_colour);
	line_info[1].point = *v1;
	line_info[1].colour = halo_colour_to_d3d_colour(end_colour);


	LOG_CHECK(get_global_d3d_device()->DrawPrimitiveUP(D3DPT_LINELIST, 1, line_info, sizeof(s_debug_vertex)) == NOERROR);
}

const colour_rgba pathfinding_debug_colour_default( 1.0f, 0.0f, 1.0f, 1.0f );

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
				colour_rgba pathfinding_debug_colour = pathfinding_debug_colour_default;
				if (conf.exists("pathfinding_color"))
				{
					colour_rgb colour;
					if (string_to_colour_rgb(conf.getString("pathfinding_color"), colour))
					{
						pathfinding_debug_colour = colour.as_rgba();
					}
				}
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
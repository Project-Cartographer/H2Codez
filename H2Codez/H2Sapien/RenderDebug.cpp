#include "RenderDebug.h"
#include <d3d9.h>
#include "..\Common\H2EKCommon.h"
#include "..\util\Patches.h"
#include "..\Tags\ScenarioStructureBSP.h"
#include "..\stdafx.h"
#include "..\util\string_util.h"
#include "..\Common\Pathfinding.h"
#include <random>

static inline scenario_structure_bsp_block *get_sbsp()
{
	return *reinterpret_cast<scenario_structure_bsp_block **>(0xA9CA74);
}

static inline IDirect3DDevice9Ex *get_global_d3d_device()
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

void draw_debug_line(const real_point3d *v0, const real_point3d *v1, 
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
pathfinding::render_info lines_to_render;

void __cdecl render_debug_info_game_in_progress()
{
	if (conf.getBoolean("render_pathfinding_debug", false) || is_debug_build())
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
				std::mt19937 gen(343 + lines_to_render.sector_lines.size()); // guaranteed to be random choosen via 400-side dice
				std::uniform_real_distribution<float> random_colour_change(-0.2f, 0.2f);

				for (auto &sector_info : lines_to_render.sector_lines)
				{
					auto mutate_channel = [&](float &colour) {
						float offset = random_colour_change(gen);
						colour += offset;
						colour = std::abs(colour);
					};
					mutate_channel(pathfinding_debug_colour.blue);
					mutate_channel(pathfinding_debug_colour.red);
					mutate_channel(pathfinding_debug_colour.green);
					mutate_channel(pathfinding_debug_colour.alpha);
					pathfinding_debug_colour.alpha = std::max(pathfinding_debug_colour.alpha, 0.5f); // make sure the sector doesn't get too dark
					pathfinding_debug_colour.clamp();
					for (auto line : sector_info.second.lines)
					{
						auto link = pathfinding->links[line];
						if (LOG_CHECK(link))
						{
							auto vertex1 = pathfinding->vertices[link->vertex1];
							auto vertex2 = pathfinding->vertices[link->vertex2];

							if (vertex1 && vertex2)
								draw_debug_line(&vertex1->point, &vertex2->point, &pathfinding_debug_colour);
						}
					}
				}
			}
		}
	}
}



void setup_engine_for_new_sbsp()
{
	lines_to_render = pathfinding::get_render_info(get_sbsp());

	typedef void __cdecl setup_engine_for_new_sbsp();
	auto setup_engine_for_new_sbsp_impl = reinterpret_cast<setup_engine_for_new_sbsp*>(0x4FF660);
	return setup_engine_for_new_sbsp_impl();
}

void H2SapienPatches::render_debug_info_init()
{
	PatchCall(0x6D31F0, render_debug_info_game_in_progress);
	PatchCall(0x4D6A63, setup_engine_for_new_sbsp);
}
#pragma once
#include "h2codez.h"

struct lightmap_quality_setting
{
	const wchar_t *name;
	uint32_t secondary_monte_carlo_setting;
	uint32_t main_monte_carlo_setting;
	uint32_t is_draft;
	uint32_t proton_count;
	uint32_t is_direct_only;
	float    unk7;
	uint32_t is_checkboard;
};
CHECK_STRUCT_SIZE(lightmap_quality_setting, 0x20);

// allows distributing lightmapping over multiple computers
void _cdecl generate_lightmaps_slave(const wchar_t *argv[]);

// starts slave and forks once we get to rasterizing
void _cdecl generate_lightmaps_fork_slave(const wchar_t *argv[]);

// merges together the distributed lightmap
void _cdecl generate_lightmaps_master(const wchar_t *argv[]);

// runs multiple lighermappers and then merges the resulting data
void _cdecl generate_lightmaps_local_multi_process(const wchar_t *argv[]);

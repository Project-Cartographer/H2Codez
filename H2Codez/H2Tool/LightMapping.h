#pragma once

// allows distributing lightmapping over multiple computers
void _cdecl generate_lightmaps_slave(const wchar_t *argv[]);

// merges together the distributed lightmap
void _cdecl generate_lightmaps_master(const wchar_t *argv[]);

// runs multiple lighermappers and then merges the resulting data
void _cdecl generate_lightmaps_local_multi_process(const wchar_t *argv[]);

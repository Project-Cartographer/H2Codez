#include "H2Tool_Commands.h"
#include "util/Patches.h"
#include "util/Numerical.h"

/* setups intial values for all lightmap/lightprobe settings */
static void lightmap_settings_init(bool setup_bsp_errors)
{
	typedef void __cdecl lightmap_settings_init(char setup_bsp_errors);
	auto lightmap_settings_init_impl = reinterpret_cast<lightmap_settings_init*>(0x4C0DC0);
	lightmap_settings_init_impl(setup_bsp_errors);
}

/* Sets all values relating to qualit from setting table */
static void process_lightmap_quality_settings(const wchar_t *quality_string)
{
	typedef void __cdecl process_lightmap_quality_settings(const wchar_t *quality_string);
	auto process_lightmap_quality_settings_impl = reinterpret_cast<process_lightmap_quality_settings*>(0x4C2990);
	process_lightmap_quality_settings_impl(quality_string);
}

/* Main function that does all lightmap related work */
static void do_light_calculations(const wchar_t *scenario_name, const wchar_t *bsp_name)
{
	DWORD do_light_calculations = 0x4C60E0;
	__asm {
		push bsp_name
		mov ecx, scenario_name
		call do_light_calculations
		add esp, 4
	}
}

enum lightmapping_distributed_type : int
{
	local,
	slave,
	master
};

struct lightmap_control
{
	size_t editable_bitmap_group;
	size_t output_bitmap_group;
	size_t radiance_bitmap_group;
	size_t lightmap_group;
	lightmapping_distributed_type distributed_type;
};

lightmap_control *global_lightmap_control = reinterpret_cast<lightmap_control*>(0xA73CE0);

static lightmapping_distributed_type global_lightmap_control_distributed_type = local;

char lightmap_log_name[0x100] = "lightmap.log";

static bool set_slave_count(const wchar_t *count)
{
	try {
		WriteValue<DWORD>(0xA73D7C, std::stoul(count, 0, numerical::get_base(wstring_to_string.to_bytes(count))));
		return true;
	}
	catch (const std::exception &e)
	{
		LOG_FUNC("Exception thrown: %s", e.what());
		return false;
	}
}

void __cdecl generate_lightmaps_slave(const wchar_t *argv[])
{
	lightmap_settings_init(true);
	process_lightmap_quality_settings(argv[2]);
	if (!LOG_CHECK(set_slave_count(argv[3])))
	{
		printf("Invalid slave count");
		return;
	}
	size_t slave_id = std::stoul(argv[4], 0, numerical::get_base(wstring_to_string.to_bytes(argv[4])));
	// lightmap slave id?
	WriteValue<DWORD>(0xA73D78, slave_id);

	sprintf_s(lightmap_log_name, "lightmap_slave_%d.log", slave_id);

	global_lightmap_control_distributed_type = slave;

	do_light_calculations(argv[0], argv[1]);
}

void __cdecl generate_lightmaps_master(const wchar_t *argv[])
{
	lightmap_settings_init(true);
	process_lightmap_quality_settings(argv[2]);
	if (!LOG_CHECK(set_slave_count(argv[3])))
	{
		printf("Invalid slave count");
		return;
	}

	global_lightmap_control_distributed_type = master;
	sprintf_s(lightmap_log_name, "lightmap_master.log");

	do_light_calculations(argv[0], argv[1]);
}

void H2ToolPatches::reenable_lightmap_farming()
{
	// hook lightmap control to work around some logging getting disabled when not in local mode
	constexpr DWORD lightmap_distributed_type_offsets[] = {
		0x4B314F, 0x4B362D, 0x4BB402, 0x4BD5CF,
		0x4BD6C1, 0x4BD877, 0x4BD9C1, 0x4E27AF,
		0x4C7206, 0x4C70D9, 0x4C6FD6, 0x4C6E50,
		0x4C6DAE, 0x4C6B77, 0x4C6A2F, 0x4C1F5F,
		0x4C1039, 0x4BFCA9, 0x4BF6FC
	};

	for (DWORD offset : lightmap_distributed_type_offsets)
		WritePointer(offset + 2, &global_lightmap_control_distributed_type);
	WritePointer(0x4C6F40 + 1, &global_lightmap_control_distributed_type);
	WritePointer(0x4C6E7F + 1, &global_lightmap_control_distributed_type);

	// Fix log name being weird due to type confusion
	WritePointer(0x4C1BDA + 1, "%s_%ws_%s");
	WritePointer(0x4C1B92 + 1, "%s\\tags\\%s_%ws_%s");

	// patch the log names to allow multiple lightmappers to work on one map
	WritePointer(0x4C1BBF + 1, lightmap_log_name);
	WritePointer(0x4C1B76 + 1, lightmap_log_name);
}

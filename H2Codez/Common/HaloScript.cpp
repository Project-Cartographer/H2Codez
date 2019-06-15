#include "HaloScript.h"
#include "HaloScript/hs_interface.h"
#include "util/Patches.h"
#include "H2EKCommon.h"

void fix_haloscript_pointers()
{
	// Replace pointers to the command table
	const static std::vector<DWORD> cmd_table_offsets_tool =
	{
		0x005C5365 + 3, 0x005C5530 + 3, 0x005C5554 + 3,
		0x005C5821 + 3, 0x005C5C44 + 3, 0x005C5E64 + 3,
		0x005C5E9A + 3, 0x005C5F48 + 3
	};
	const static std::vector<DWORD> cmd_table_offsets_sapien =
	{
		0x004E2225 + 3, 0x004E23F0 + 3,  0x004E2414 + 3,
		0x004E2701 + 3, 0x004E2DF4 + 3, 0x004E3014 + 3,
		0x004E304D,     0x004E30FB
	};
	std::vector<DWORD> &cmd_table_offsets = SwitchByMode(cmd_table_offsets_tool, cmd_table_offsets_sapien, {});

	const hs_command **cmds = g_halo_script_interface->get_command_table();

	for (DWORD addr : cmd_table_offsets)
		WritePointer(addr, cmds);

	// patch command table size
	const static int hs_cmd_table_size = g_halo_script_interface->get_command_table_count();
	WriteValue(SwitchByMode(0x008CD59C, 0x008EB118, NULL), hs_cmd_table_size);

	// Replace pointers to the globals table

	const static std::vector<DWORD> var_table_offsets_tool =
	{
		0x005C53D5, 0x005C53F0, 0x005C5430,
		0x005C5474, 0x005C58D1, 0x006884A1,
		0x006884BD, 0x0068850D, 0x0068858B,
		0x006885A2
	};
	const static std::vector<DWORD> var_table_offsets_sapien =
	{
		0x004E2295, 0x004E22B0, 0x004E22F0,
		0x004E2334, 0x004E27B1, 0x00635A11,
		0x00635A2D, 0x00635A7D, 0x00635AFB,
		0x00635B12
	};

	std::vector<DWORD> &var_table_offsets = SwitchByMode(var_table_offsets_tool, var_table_offsets_sapien, {});

	const hs_global_variable **vars = g_halo_script_interface->get_global_table();

	for (DWORD addr : var_table_offsets)
		WriteValue(addr + 3, vars);

	// patch globals table size
	const static int hs_global_table_size = g_halo_script_interface->get_global_table_count();
	WriteValue(SwitchByMode(0x008D2238, 0x008EFDB4, NULL), hs_global_table_size);
}

bool wake_hs_thread_by_name(char *thread)
{
	typedef char __cdecl _wake_hs_thread_by_name(char *thread);
	auto _wake_hs_thread_by_name_impl = reinterpret_cast<_wake_hs_thread_by_name*>(0x52C5B0);
	return _wake_hs_thread_by_name_impl(thread);
}

void init_custom_commands()
{
#pragma region extensions
	hs_custom_command enable_custom_script_sync("enable_custom_script_sync", "Allows running scripts on client using wake_sync (extension function).", NULL_HS_FUNC); // does nothing in sapien
	g_halo_script_interface->RegisterCustomCommand(hs_opcode::enable_custom_script_sync, enable_custom_script_sync);

	struct start_sync_args
	{
		char *script_name;
	};
	auto start_sync_func = HS_FUNC(
		wake_hs_thread_by_name((static_cast<start_sync_args*>(args))->script_name);
		return 0;
		);
	hs_custom_command wake_sync("wake_sync",
		"Run a script on both server and clients (extension function).",
		start_sync_func,
		{ hs_type::string }
	);
	g_halo_script_interface->RegisterCustomCommand(hs_opcode::wake_sync, wake_sync);
#pragma endregion

	struct set_temp_args
	{
		char *setting;
		char *value;
	};
	hs_custom_command set_temp(
		"set_temp",
		"Sets temporally setting",
		HS_FUNC(
			auto info = static_cast<set_temp_args*>(args);
			return conf.setTempSetting(info->setting, info->value);
		),
		{ hs_type::string , hs_type::string },
		hs_type::boolean,
		"<string:setting> <string:value>"
	);
	// already a nop in both game and sapien so safe to reuse.
	g_halo_script_interface->RegisterCustomCommand(hs_opcode::test_network_storage_simulate, set_temp);

	const hs_custom_command script_doc_cmd(
		"script_doc",
		"saves a file called hs_doc.txt with parameters for all script commands and globals.",
		HS_FUNC(
			H2CommonPatches::generate_script_doc("hs_doc.txt");
			return 0;
		)
	);
	g_halo_script_interface->RegisterCustomCommand(hs_opcode::script_doc, script_doc_cmd);
}

void init_custom_variables()
{
	static int api_version = 1;

	constexpr static hs_global_variable api_extension_version
	(
		"api_extension_version",
		hs_type::hs_long,
		&api_version
	);
	g_halo_script_interface->RegisterGlobal(hs_global_id::api_extension_version, &api_extension_version);
}

static short numbers_a[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
static short numbers_b[] = { 0, 1, 2, 3, 4, 5, 6 };

void H2CommonPatches::haloscript_init()
{
	// init tables
	hs_command **command_table = reinterpret_cast<hs_command **>(SwitchByMode(0x009ECFE0, 0x9E9E90, 0x95BF70));
	hs_global_variable **global_table = reinterpret_cast<hs_global_variable **>(SwitchByMode(0x009EFF78, 0x9ECE28, 0x95EF08));
	g_halo_script_interface->init_custom(command_table, global_table);

	static constexpr hs_global_variable radar_global{ "some_radar_thing", hs_type::boolean };
	g_halo_script_interface->set_global(hs_global_id::some_radar_thing, &radar_global);

	static constexpr hs_global_variable ai_current_squad{ "ai_current_squad", hs_type::ai };
	g_halo_script_interface->set_global(hs_global_id::ai_current_squad, &ai_current_squad);
	static constexpr hs_global_variable ai_current_actor{ "ai_current_actor", hs_type::ai };
	g_halo_script_interface->set_global(hs_global_id::ai_current_actor, &ai_current_actor);

	// this is ugly but it will have to do
	for (size_t i = 0; i < ARRAYSIZE(numbers_a); i++)
	{
		std::string *name = new std::string("numa_" + std::to_string(i));
		hs_global_variable *var = new hs_global_variable(name->c_str(), hs_type::hs_short, &numbers_a[i]);
		g_halo_script_interface->global_table[static_cast<size_t>(hs_global_id::num_a_0) + i] = var;
	}
	
	for (size_t i = 0; i < ARRAYSIZE(numbers_b); i++)
	{
		std::string *name = new std::string("numb_" + std::to_string(i));
		hs_global_variable *var = new hs_global_variable(name->c_str(), hs_type::hs_short, &numbers_a[i]);
		g_halo_script_interface->global_table[static_cast<size_t>(hs_global_id::num_b_0) + i] = var;
	}

	static constexpr hs_global_variable unk_bool{ "unk_bool", hs_type::boolean };
	g_halo_script_interface->set_global(hs_global_id::unk_bool, &unk_bool);
	static constexpr hs_global_variable unk_bool2{ "unk_bool_2", hs_type::boolean };
	g_halo_script_interface->set_global(hs_global_id::unk_bool_2, &unk_bool2);
	static constexpr hs_global_variable unk_bool_hud{ "unk_bool_hud", hs_type::boolean };
	g_halo_script_interface->set_global(hs_global_id::unk_bool_hud, &unk_bool_hud);

	// unknown extra commands in the game binary
#pragma region unknown nops
	hs_custom_command unknown_stub(
		"unknown_command",
		"Does nothing.",
		NULL_HS_FUNC
	);
	g_halo_script_interface->RegisterCustomCommand(hs_opcode::hs_unk_1, unknown_stub);
	g_halo_script_interface->RegisterCustomCommand(hs_opcode::hs_unk_2, unknown_stub);
	g_halo_script_interface->RegisterCustomCommand(hs_opcode::hs_unk_3, unknown_stub);
	g_halo_script_interface->RegisterCustomCommand(hs_opcode::hs_unk_4, unknown_stub);
#pragma endregion

	fix_haloscript_pointers();
	init_custom_commands();
	init_custom_variables();
};

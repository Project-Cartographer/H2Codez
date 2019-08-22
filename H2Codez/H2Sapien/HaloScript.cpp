#include "HaloScript.h"
#include "SapienInterface.h"
#include "Common/Pathfinding.h"
#include "Common/H2EKCommon.h"
#include "Common/TagInterface.h"
#include "Common/GlobalTags.h"
#include "HaloScript/hs_interface.h"
#include <Shellapi.h>

using namespace SapienInterface;

void status_dump()
{
	ofstream output;
	std::string temp_file_name = H2CommonPatches::get_temp_name("status.txt");

	output.open(temp_file_name, ios::out);
	if (output)
	{
		auto output_variable = [&](const std::string &name, hs_type type, void *data)
		{
			std::string value_as_string = get_value_as_string(data, type);
			output << setw(60) << name << "    :    " << value_as_string << std::endl;
		};

		output << "=== Built in Globals ===\n" << std::endl;
		for (auto *current_var : g_halo_script_interface->global_table)
		{
			if (current_var->type == hs_type::nothing) // used for padding, don't actual do anything
				continue;
			output_variable(current_var->name, current_var->type, current_var->variable_ptr);
		}

		output << "\n=== Script Globals ===\n" << std::endl;

		auto global_scenario = GlobalTags::get_scenario();
		auto script_nodes = get_script_node_array();
		for (size_t index = 0; index < global_scenario->globals.size; index++)
		{
			auto global = global_scenario->globals[index];
			if (!global)
				break;
			auto *variable = script_nodes->datum_get<s_script_global>(static_cast<WORD>(index + HaloScriptInterface::get_global_table_count()));

			output_variable(global->name, global->type, &variable->value);
		}
	}
	output.close();
	ShellExecuteA(NULL, NULL, temp_file_name.c_str(), NULL, NULL, SW_SHOW);
}

void H2SapienPatches::haloscript_init()
{
	const hs_custom_command status_cmd(
		"status",
		"Dumps the value of all global status variables to file.",
		HS_FUNC(
			status_dump();
			return 0;
		)
	);
	g_halo_script_interface->RegisterCustomCommand(hs_opcode::status, status_cmd);

	const hs_custom_command pathfinding_cmd(
		"generate_pathfinding",
		"Generate pathfinding from collision for current BSP",
		HS_FUNC(
			auto bsp = get_sbsp_index();
			auto bsp_data = tags::get_tag<scenario_structure_bsp_block>('sbsp', bsp);
			if (bsp_data->pathfindingData.size == 0)
				pathfinding::generate(bsp);
			return 0;
		)
	);
	g_halo_script_interface->RegisterCustomCommand(hs_opcode::generate_pathfinding, pathfinding_cmd);
}
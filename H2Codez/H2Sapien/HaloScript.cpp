#include "HaloScript.h"
#include "../Common/Pathfinding.h"
#include "../Common/H2EKCommon.h"
#include "../HaloScript/hs_interface.h"
#include <Shellapi.h>

void status_dump()
{
	ofstream output;
	std::string temp_file_name = H2CommonPatches::get_temp_name("status.txt");

	output.open(temp_file_name, ios::out);
	if (output)
	{
		for (hs_global_variable *current_var : g_halo_script_interface->global_table)
		{
			std::string value_as_string = get_value_as_string(current_var->variable_ptr, current_var->type);
			output << current_var->name << "   :    " << value_as_string << std::endl;
		}
	}
	output.close();
	ShellExecuteA(NULL, NULL, temp_file_name.c_str(), NULL, NULL, SW_SHOW);
}

static inline scenario_structure_bsp_block *get_sbsp()
{
	return *reinterpret_cast<scenario_structure_bsp_block **>(0xA9CA74);
}

void H2SapienPatches::haloscript_init()
{
	hs_custom_command status_cmd(
		"status",
		"Dumps the value of all global status variables to file.",
		HS_FUNC(
			status_dump();
			return 0;
		)
	);
	g_halo_script_interface->RegisterCustomCommand(hs_opcode::status, status_cmd);

	hs_custom_command pathfinding_cmd(
		"generate_pathfinding",
		"Generate pathfinding from collision for current bsp",
		HS_FUNC(
			auto bsp = get_sbsp();
			if (bsp->pathfindingData.size == 0)
				pathfinding::generate(bsp);
			return 0;
		)
	);
	g_halo_script_interface->RegisterCustomCommand(hs_opcode::generate_pathfinding, pathfinding_cmd);
}
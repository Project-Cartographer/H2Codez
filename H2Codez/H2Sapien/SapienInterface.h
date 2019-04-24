#pragma once
#include "Tags\ScenarioStructureBSP.h"
#include "Tags\ScenarioTag.h"
#include "util\CriticalSection.h"
#include "Common\data\data_array.h"

namespace SapienInterface
{
	inline scnr_tag **get_global_scenario_ptr()
	{
		return reinterpret_cast<scnr_tag**>(0xA9CA7C);
	}

	inline scnr_tag *get_global_scenario()
	{
		return *get_global_scenario_ptr();
	}

	inline scenario_structure_bsp_block **get_global_structure_bsp_ptr()
	{
		return reinterpret_cast<scenario_structure_bsp_block**>(0xA9CA74);
	}

	inline scenario_structure_bsp_block *get_global_structure_bsp()
	{
		return *get_global_structure_bsp_ptr();
	}

	inline WORD *get_sbsp_index_pointer()
	{
		return reinterpret_cast<WORD*>(0x9A14B8);
	}

	inline WORD get_sbsp_index()
	{
		return *get_sbsp_index_pointer();
	}

	inline s_data_array **get_script_node_array_ptr()
	{
		return reinterpret_cast<s_data_array**>(0xA9CC14);
	}

	inline s_data_array *get_script_node_array()
	{
		return *get_script_node_array_ptr();
	}

	bool load_structure_bsp(int bsp_block_index, bool unload_old = true);

	// broken, fix before using
	inline bool reload_structure_bsp()
	{
		static CriticalSection critical_section;
		bool success = false;
		if (critical_section.try_enter())
		{
			// save old sbsp index
			auto sbsp_index = get_sbsp_index();
			success = LOG_CHECK(load_structure_bsp(NONE)); // unload bsp
			success = LOG_CHECK(load_structure_bsp(sbsp_index)) && success; // reload bsp
			critical_section.leave();
		}
		return success;
	}
};

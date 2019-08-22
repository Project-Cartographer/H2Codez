#pragma once
#include "Tags\ScenarioTag.h"

namespace GlobalTags
{
	inline scnr_tag** get_scenario_pointer()
	{
		return reinterpret_cast<scnr_tag**>(SwitchAddessByMode(0xAA00E4, 0xA9CA7C, 0));
	}

	inline scnr_tag* get_scenario()
	{
		auto **scrn_ptr = get_scenario_pointer();
		CHECK_FUNCTION_SUPPORT(scrn_ptr);
		return *scrn_ptr;
	}
}

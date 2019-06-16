#pragma once
#include "util/array.h"
#include "hs_global_ids.h"

namespace HaloScriptCommon {
#define entry(...) \
	std::pair<HaloScriptCommon::hs_global_id, const char*>{ __VA_ARGS__ }

	constexpr static_map<8, hs_global_id, const char*> hs_descriptions
	{
		entry(hs_global_id::debug_disable_radar_fade, "Disable the dot fade shader with this global"),
		entry(hs_global_id::ai_movement_patrol, "Use this global with cs_movement_mode to set AI or squad to patrol the area. Needs to be set up to patrol to do this."),
		entry(hs_global_id::ai_movement_sleep, "Use this global with cs_movement_mode to make the AI or squad sleep."),
		entry(hs_global_id::ai_movement_combat, "Use this global with cs_movement_mode to make the AI or squad enter combat."),
		entry(hs_global_id::ai_movement_flee, "Use this global with cs_movement_mode to make the AI or squad flee. Needs to transition into this."),
		entry(hs_global_id::ai_movement_panic, "Use this global with cs_movement_mode to make the AI or squad act suprised."),
		entry(hs_global_id::ai_movement_t_pose, "Use this global with cs_movement_mode to make the AI or squad cower in place."),
		entry(hs_global_id::ai_movement_flee_b, "Same as ai_movement_flee ??"),
	};

#undef entry
}

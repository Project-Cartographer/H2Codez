#include "HaloScript.h"
#include "Common\H2EKCommon.h"
#include "Common\data\data_array.h"
#include "Common\HaloScriptInterface.h"
#include "Common\GlobalTags.h"
#include "HaloScript\hs_ai_type.h"
#include "Tags\ScenarioTag.h"
#include "Util\string_util.h"
#include "Util\numerical.h"
#include "Util\array.h"

/*
	Fixes the haloscript compiler not generating the correct data for some types.
*/

using namespace HaloScriptCommon;

/* 
	Sets the syntax node's value to the index of the element which contains the string id
*/
static void hs_convert_string_id_to_tagblock_index(tag_block_ref *tag_block, int block_offset, int script_node_index)
{
	auto script_node = hs_get_script_node(script_node_index);
	const char *value_string = hs_get_string_data(script_node);

	script_node->value = tag_block->find_string_id_element(block_offset, string_id::find_by_name(value_string));

	if (script_node->value == NONE) {
		std::string error = "this is not a valid '" + hs_type_string[static_cast<hs_type>(script_node->value_type)] + "' name, check tags";
		hs_parser_error(script_node, error);
	}
}

/*
	Sets the syntax node's value to the index of the element which contains the string
*/
static void hs_convert_string_to_tagblock_offset(tag_block_ref *tag_block, int block_offset, int script_node_index)
{
	auto script_node = hs_get_script_node(script_node_index);
	const char *value_string = hs_get_string_data(script_node);

	script_node->value = tag_block->find_string_element(block_offset, value_string);

	if (script_node->value == NONE) {
		std::string error = "this is not a valid '" + get_hs_type_string(script_node->value_type) + "' name, check tags";
		hs_parser_error(script_node, error);
	}
}

static char __cdecl hs_parse_conversation(unsigned __int16 script_node_index)
{
	scnr_tag *scenario = GlobalTags::get_scenario();
	hs_convert_string_to_tagblock_offset(&scenario->aIConversations, 0, script_node_index);
	return 1;
}

template <size_t min = 0, size_t max = MAXDWORD>
static char __cdecl hs_raw_id_passthrough(unsigned __int16 index)
{
	hs_script_node *script_node = hs_get_script_node(index);
	const char *input_string = hs_get_string_data(script_node);

	auto report_out_of_range = [&]() {
		hs_parser_error(script_node,
			get_hs_type_string(script_node->value_type) + " ID out of range [" + std::to_string(min) + "->" + std::to_string(max) + "]");
	};

	if (!_stricmp(input_string, "NONE")) {
		script_node->value = NONE;
		return true;
	}
	try {
		size_t as_number = std::stoul(input_string, nullptr, 0);
		if (numerical::is_between(as_number, min, max))
		{
			script_node->value = as_number;
			return true;
		}
		report_out_of_range();
		return false;
	}
	catch (std::invalid_argument&) {
		hs_parser_error(script_node, "invalid " + get_hs_type_string(script_node->value_type) + " ID");
		return false;
	}
	catch (std::out_of_range&)
	{
		report_out_of_range();
		return false;
	}
}

static char __cdecl hs_parse_ai_behaviour(unsigned __int16 script_node_index)
{
	hs_script_node *script_node = hs_get_script_node(script_node_index);
	const std::string input = hs_get_string_data(script_node);
	ai_behaviour out = string_to_ai_behaviour(input);
	if (out != ai_behaviour::invalid) {
		script_node->value = static_cast<DWORD>(out);
		return true;
	}
	else {
		hs_parser_error(script_node, "Invalid AI behavior");
		return false;
	}
}

static char __cdecl hs_parse_ai_orders(unsigned __int16 script_node_index)
{
	scnr_tag *scenario = GlobalTags::get_scenario();
	hs_convert_string_to_tagblock_offset(&scenario->orders, 0, script_node_index);
	return true;
}

static char __cdecl hs_parse_ai(unsigned __int16 script_node_index)
{
	scnr_tag *scenario = GlobalTags::get_scenario();
	hs_script_node *script_node = hs_get_script_node(script_node_index);
	std::string input_string = hs_get_string_data(script_node);

	hs_ai_type ai{};

	if (input_string.find('/') != std::string::npos) {
		std::string squad_name = input_string.substr(0, input_string.find('/'));
		std::string squad_pos = input_string.substr(input_string.find('/') + 1);

		size_t squads_index = scenario->squads.find_string_element(offsetof(squads_block, name), squad_name);
		if (squads_index != NONE) {
			auto *squad = ASSERT_CHECK(scenario->squads[squads_index]);

			uint32_t location_idx = squad->startingLocations.find_string_id_element(offsetof(actor_starting_locations_block, name), squad_pos);
			if (location_idx != NONE) {
				ai.set_starting_location(squads_index, location_idx);
			} else if (is_string_numerical(squad_pos)) { // temp hack to support old style HS code
				ai.set_starting_location(squads_index, static_cast<uint32_t>(strtol(squad_pos.c_str(), nullptr, 0)));
			} else {
				hs_parser_error(script_node, "No such starting location.");
				return false;
			}
		} else {
			hs_parser_error(script_node, "No such squad.");
			return false;
		}
	} else {
		size_t squads_index = scenario->squads.find_string_element(offsetof(squads_block, name), input_string);
		size_t squads_group_index = scenario->squadGroups.find_string_element(offsetof(squad_groups_block, name), input_string);

		// attempt to find a squad with that name first before trying squads groups
		if (squads_index != NONE)
			ai.set_squad(squads_index);
		else if (squads_group_index != NONE) 
			ai.set_squad_group(squads_group_index);
	}

	if (!ai.is_type_set())
	{
		hs_parser_error(script_node, "Failed to parse AI.");
		return false;
	}
		
	script_node->value = ai.get_packed();

	return true;
}

static char __cdecl hs_parse_point_ref(unsigned __int16 script_node_index)
{
	scnr_tag *scenario = GlobalTags::get_scenario();
	hs_script_node *script_node = hs_get_script_node(script_node_index);
	std::string input_string = hs_get_string_data(script_node);
	auto scripting_data = scenario->scriptingData[0];

	if (!scripting_data) {
		hs_parser_error(script_node, "Script data missing from scenario.");
		return false;
	}

	if (input_string.find('/') != std::string::npos) {
		std::string point_set = input_string.substr(0, input_string.find('/'));
		std::string point = input_string.substr(input_string.find('/') + 1);

		size_t point_set_index = scripting_data->pointSets.find_string_element(offsetof(cs_point_set_block, name), point_set);
		if (point_set_index != NONE) {
			cs_point_set_block *points_set = scripting_data->pointSets[point_set_index];
			size_t point_index = points_set->points.find_string_element(offsetof(cs_point_block, name), point);

			if (point_index != NONE) {
				script_node->value = (point_set_index << 16 | point_index);
				return true;
			} else {
				hs_parser_error(script_node, "No such point.");
				return false;
			}
		}
		else {
			hs_parser_error(script_node, "No such point set.");
			return false;
		}
	}
	else {
		size_t point_set_index = scripting_data->pointSets.find_string_element(offsetof(cs_point_set_block, name), input_string);
		if (point_set_index != NONE) {
			script_node->value = (0xFFFF << 16 | point_set_index);
			return true;
		} else {
			hs_parser_error(script_node, "No such point set.");
			return false;
		}
	}
}

static char __cdecl hs_parse_navpoint(unsigned __int16 script_node_index)
{
	constexpr static char *way_point_names[]
	{
		"default",
		"ctf_flag",
		"assault_bomb",
		"ally",
		"dead_ally",
		"oddball_ball",
		"king_hill",
		"territories_territory"
	};

	auto normalize = [](int ch) -> int {
		if (std::isalnum(ch))
			return std::tolower(ch);
		return '_';
	};

	hs_script_node *script_node = hs_get_script_node(script_node_index);
	std::string input_string = hs_get_string_data(script_node);

	std::string navpoint_string = transform_string(str_trim(input_string), normalize);
	for (size_t i = 0; i < ARRAYSIZE(way_point_names); i++)
	{
		if (navpoint_string == way_point_names[i])
		{
			script_node->value = i;
			return true;
		}
	}
	return hs_raw_id_passthrough<0, 7>(script_node_index);
}

struct hs_parser_enum_info
{
	size_t count;
	const char *const*values;
};

#define entry(enum, enum_values) \
	std::pair<hs_type, hs_parser_enum_info>{enum,  hs_parser_enum_info{ARRAYSIZE(enum_values), enum_values} }

constexpr const static char *game_difficulty_enum_values[]{ "easy", "normal", "heroic", "legendary" };
constexpr const static char *team_enum_values[]
{
	"default", "player", "human", "covenant", "flood", "sentinel", "heretic", "prophet",
	"unused8", "unused9", "unused10", "unused11", "unused12", "unused13", "unused14", "unused15"
};
constexpr const static char *actor_type_enum_values[]
{
	"elite",
	"jackal",
	"grunt",
	"hunter",
	"engineer",
	"assassin",
	"player",
	"marine",
	"crew",
	"combat_form",
	"infection_form",
	"carrier_form",
	"monitor",
	"sentinel",
	"none",
	"mounted_weapon",
	"brute",
	"prophet",
	"bugger"
};

constexpr const static char *hud_corner_enum_values[]{ "top_left", "top_right", "top_right", "bottom_right", "center" };
constexpr const static char *model_state_enum_values[]{ "standard", "minor damage", "medium damage", "major damage", "destroyed" };
constexpr const static char *network_event_enum_values[]{ "verbose", "status", "message", "warning", "error", "critical" };


static static_map<6, hs_type, hs_parser_enum_info> hs_enum_values
{
	entry(hs_type::game_difficulty, game_difficulty_enum_values),
	entry(hs_type::team, team_enum_values),
	entry(hs_type::actor_type, actor_type_enum_values),
	entry(hs_type::hud_corner, hud_corner_enum_values),
	entry(hs_type::model_state, model_state_enum_values),
	entry(hs_type::network_event, model_state_enum_values)
};

#undef entry

static char __cdecl hs_parse_enum(unsigned __int16 script_node_index)
{
	hs_script_node *script_node = hs_get_script_node(script_node_index);
	if (script_node->value_type < hs_type::game_difficulty || script_node->value_type > hs_type::network_event &&
		script_node->value_type != script_node->constant_type)
	{
		hs_parser_errorf(script_node, "corrupt enum expression(type %d constant - type % d)", script_node->value_type, script_node->constant_type);
		return false;
	}

	const char *enum_string = hs_get_string_data(script_node);
	try {
		const auto &enum_values = hs_enum_values[script_node->value_type];
		for (size_t idx = 0; idx < enum_values.count; idx++)
		{
			if (_strcmpi(enum_values.values[idx], enum_string) == 0)
			{
				script_node->value = idx;
				return true;
			}
		}

		std::stringstream ss;
		ss << get_hs_type_string(script_node->value_type) << " must be ";
		if (enum_values.count > 1)
		{
			for (size_t idx = 0; idx < enum_values.count - 1; idx++)
				ss << "\"" << enum_values.values[idx] << "\", ";
			ss << "or ";
		}
		ss << "\"" << enum_values.values[enum_values.count] << "\".";
		hs_parser_error(script_node, ss.str());
		return false;
	} catch (const std::exception &ex) {
		hs_parser_error(script_node, "Internal error");
		LOG_FUNC("exception::what = %", ex.what());
		return false;
	}
}

#define set_hs_converter(type, func) \
	hs_parser_lookup_table[static_cast<int>(type)] = func;

void H2CommonPatches::fix_hs_converters()
{
	void **hs_parser_lookup_table = hs_get_type_parser_table();
	CHECK_FUNCTION_SUPPORT(hs_parser_lookup_table);
	set_hs_converter(hs_type::ai_behavior, hs_parse_ai_behaviour);
	set_hs_converter(hs_type::conversation, hs_parse_conversation);
	set_hs_converter(hs_type::ai_orders, hs_parse_ai_orders);
	set_hs_converter(hs_type::ai, hs_parse_ai);
	set_hs_converter(hs_type::point_reference, hs_parse_point_ref);
	set_hs_converter(hs_type::navpoint, hs_parse_navpoint);
	// set style to use a tag converter
	set_hs_converter(hs_type::style, hs_parser_lookup_table[static_cast<int>(hs_type::sound)]);

	// hacky workaround, lets the user directly input the ID it's meant to generate.
	hs_type passthrough_types[] = {
		hs_type::hud_message,
		hs_type::ai_command_list
	};

	for (auto i : passthrough_types)
		set_hs_converter(i, hs_raw_id_passthrough<>);

	for (int type = static_cast<int>(hs_type::game_difficulty); type < static_cast<int>(hs_type::network_event); type++)
		set_hs_converter(type, hs_parse_enum);
}

#undef set_hs_converter

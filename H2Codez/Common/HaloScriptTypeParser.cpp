#include "HaloScript.h"
#include "Common\H2EKCommon.h"
#include "Common\data\data_array.h"
#include "Common\HaloScriptInterface.h"
#include "Common\GlobalTags.h"
#include "HaloScript\hs_ai_type.h"
#include "Tags\ScenarioTag.h"
#include "Util\string_util.h"
#include "Util\numerical.h"

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

static char __cdecl hs_convert_conversation(unsigned __int16 script_node_index)
{
	scnr_tag *scenario = GlobalTags::get_scenario();
	hs_convert_string_to_tagblock_offset(&scenario->aIConversations, 0, script_node_index);
	return 1;
}

template <size_t min = 0, size_t max = MAXDWORD>
static char __cdecl hs_convert_internal_id_passthrough(unsigned __int16 index)
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
	catch (invalid_argument) {
		hs_parser_error(script_node, "invalid " + get_hs_type_string(script_node->value_type) + " ID");
		return false;
	}
	catch (out_of_range)
	{
		report_out_of_range();
		return false;
	}
}

static char __cdecl hs_convert_ai_behaviour(unsigned __int16 script_node_index)
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

static char __cdecl hs_convert_ai_orders(unsigned __int16 script_node_index)
{
	scnr_tag *scenario = GlobalTags::get_scenario();
	hs_convert_string_to_tagblock_offset(&scenario->orders, 0, script_node_index);
	return true;
}

static char __cdecl hs_convert_ai(unsigned __int16 script_node_index)
{
	scnr_tag *scenario = GlobalTags::get_scenario();
	hs_script_node *script_node = hs_get_script_node(script_node_index);
	std::string input_string = hs_get_string_data(script_node);

	hs_ai_type ai{};

	if (input_string.find('/') != string::npos) {
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

static char __cdecl hs_convert_point_ref(unsigned __int16 script_node_index)
{
	scnr_tag *scenario = GlobalTags::get_scenario();
	hs_script_node *script_node = hs_get_script_node(script_node_index);
	std::string input_string = hs_get_string_data(script_node);
	auto scripting_data = scenario->scriptingData[0];

	if (!scripting_data) {
		hs_parser_error(script_node, "Script data missing from scenario.");
		return false;
	}

	if (input_string.find('/') != string::npos) {
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

static char __cdecl hs_convert_navpoint(unsigned __int16 script_node_index)
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
	return hs_convert_internal_id_passthrough<0, 7>(script_node_index);
}

#define set_hs_converter(type, func) \
	hs_convert_lookup_table[static_cast<int>(type)] = func;

void H2CommonPatches::fix_hs_converters()
{
	void **hs_convert_lookup_table = hs_get_type_parser_table();
	CHECK_FUNCTION_SUPPORT(hs_convert_lookup_table);
	set_hs_converter(hs_type::ai_behavior, hs_convert_ai_behaviour);
	set_hs_converter(hs_type::conversation, hs_convert_conversation);
	set_hs_converter(hs_type::ai_orders, hs_convert_ai_orders);
	set_hs_converter(hs_type::ai, hs_convert_ai);
	set_hs_converter(hs_type::point_reference, hs_convert_point_ref);
	set_hs_converter(hs_type::navpoint, hs_convert_navpoint);
	// set style to use a tag converter
	set_hs_converter(hs_type::style, hs_convert_lookup_table[static_cast<int>(hs_type::sound)]);

	// hacky workaround, lets the user directly input the ID it's meant to generate.
	hs_type passthrough_types[] = {
		hs_type::style,
		hs_type::hud_message,
		hs_type::ai_command_list
	};

	for (auto i : passthrough_types)
		set_hs_converter(i, hs_convert_internal_id_passthrough<>);
}

#undef set_hs_converter

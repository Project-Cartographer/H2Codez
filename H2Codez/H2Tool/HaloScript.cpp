#include "H2Tool_Commands.h"
#include "Common\H2EKCommon.h"
#include "Tags\ScenarioTag.h"
#include "H2ToolLibrary.inl"

hs_convert_data_store *hs_get_converter_data_store(unsigned __int16 handle)
{
	typedef void* (__cdecl *get_args)(int a1, unsigned __int16 a2);
	get_args get_args_impl = reinterpret_cast<get_args>(0x00557E60);
	return static_cast<hs_convert_data_store*>(get_args_impl(*reinterpret_cast<DWORD*>(0xBCBF4C), handle));
}

const char *hs_get_string_data(hs_convert_data_store *data_store)
{
	const char *hs_string_data = *reinterpret_cast<const char **>(0x00CDB198);
	return &hs_string_data[data_store->string_value_offset];
}

char hs_error[0x1024];

void hs_converter_error(hs_convert_data_store *data_store, const std::string &error)
{
	const char **hs_error_string_ptr = reinterpret_cast<const char**>(0x00CDB1AC);
	DWORD *hs_error_offset_ptr = reinterpret_cast<DWORD*>(0x00CDB1B0);

	strncpy(hs_error, error.c_str(), sizeof(hs_error));

	*hs_error_string_ptr = hs_error;
	*hs_error_offset_ptr = data_store->string_value_offset;
	data_store->output = NONE;
}

scnr_tag *get_global_scenario()
{
	return *reinterpret_cast<scnr_tag **>(0x00AA00E4);
}

void hs_convert_string_id_to_tagblock_offset(tag_block_ref *tag_block, int element_size, int block_offset, int hs_converter_id)
{
	auto data_store = hs_get_converter_data_store(hs_converter_id);
	const char *value_string = hs_get_string_data(data_store);

	data_store->output = FIND_TAG_BLOCK_STRING_ID(tag_block, element_size, block_offset, GET_STRING_ID(value_string));

	if (data_store->output == NONE) {
		std::string error = "this is not a valid '" + hs_type_string[static_cast<hs_type>(data_store->target_hs_type)] + "' name, check tags";
		hs_converter_error(data_store, error);
	}
}

void hs_convert_string_to_tagblock_offset(tag_block_ref *tag_block, int element_size, int block_offset, int hs_converter_id)
{
	auto data_store = hs_get_converter_data_store(hs_converter_id);
	const char *value_string = hs_get_string_data(data_store);

	data_store->output = FIND_TAG_BLOCK_STRING(tag_block, element_size, block_offset, value_string);

	if (data_store->output == NONE) {
		std::string error = "this is not a valid '" + hs_type_string[static_cast<hs_type>(data_store->target_hs_type)] + "' name, check tags";
		hs_converter_error(data_store, error);
	}
}

char __cdecl hs_convert_conversation(unsigned __int16 a1)
{
	scnr_tag *scenario = get_global_scenario();
	hs_convert_string_to_tagblock_offset(&scenario->aIConversations, 128, 0, a1);
	return 1;
}

char __cdecl hs_convert_internal_id_passthrough(unsigned __int16 a1)
{
	hs_convert_data_store *data_store = hs_get_converter_data_store(a1);
	const char *input_string = hs_get_string_data(data_store);
	if (!_stricmp(input_string, "NONE")) {
		data_store->output = NONE;
		return 1;
	}
	try {
		data_store->output = std::stoi(input_string, nullptr, 0);
		return 1;
	}
	catch (invalid_argument) {
		hs_converter_error(data_store, "invalid " + get_hs_type_string(data_store->target_hs_type) + " ID");
		return 0;
	}
	catch (out_of_range)
	{
		hs_converter_error(data_store, get_hs_type_string(data_store->target_hs_type) + " ID out of range");
		return 0;
	}
}

char __cdecl hs_convert_ai_behaviour(unsigned __int16 a1)
{
	hs_convert_data_store *data_store = hs_get_converter_data_store(a1);
	const std::string input = hs_get_string_data(data_store);
	ai_behaviour out = string_to_ai_behaviour(input);
	if (out != ai_behaviour::invalid) {
		data_store->output = static_cast<DWORD>(out);
		return 1;
	}
	else {
		hs_converter_error(data_store, "Invalid AI behaviour");
		return 0;
	}
}

char __cdecl hs_convert_ai_orders(unsigned __int16 a1)
{
	scnr_tag *scenario = get_global_scenario();
	hs_convert_string_to_tagblock_offset(&scenario->orders, 144, 0, a1);
	return 1;
}

enum ai_id_type
{
	squad,
	squad_group,
	unknown,
	starting_location,

	none = NONE
};

char __cdecl hs_convert_ai(unsigned __int16 a1)
{
	scnr_tag *scenario = get_global_scenario();
	hs_convert_data_store *data_store = hs_get_converter_data_store(a1);
	std::string input_string = hs_get_string_data(data_store);

	ai_id_type ai_type = ai_id_type::none;

	DWORD main_index = NONE;
	DWORD secondary_index = 0;

	if (input_string.find('/') != string::npos) {
		ai_type = ai_id_type::starting_location;
		std::string squad_name = input_string.substr(0, input_string.find('/'));
		std::string squad_pos = input_string.substr(input_string.find('/') + 1);

		secondary_index = FIND_TAG_BLOCK_STRING(&scenario->squads, 120, 0, squad_name);
		if (secondary_index != NONE)
		{
			// can't get the sqaud block struct working so this is a workaround for now
			main_index = strtol(squad_pos.c_str(), nullptr, 0);
		}
		else {
			hs_converter_error(data_store, "No such squad.");
			return 0;
		}
	}
	else {

		ai_type = ai_id_type::squad;
		// attempt to find a squad with that name first
		main_index = FIND_TAG_BLOCK_STRING(&scenario->squads, 120, 0, input_string);
		if (main_index == NONE) {
			// if no sqaud with that name exists try the sqaud groups
			ai_type = ai_id_type::squad_group;
			main_index = FIND_TAG_BLOCK_STRING(&scenario->squadGroups, 36, 0, input_string);
		}
	}
	if (main_index != NONE) {
		DWORD id = (ai_type << 30) | (secondary_index << 16) | main_index;
		data_store->output = id;
		return 1;
	}
	else {
		// fallback to passthrough for backwards compatibility and AI squad starting locations
		return hs_convert_internal_id_passthrough(a1);
	}
}

char __cdecl hs_convert_point_ref(unsigned __int16 a1)
{
	scnr_tag *scenario = get_global_scenario();
	hs_convert_data_store *data_store = hs_get_converter_data_store(a1);
	std::string input_string = hs_get_string_data(data_store);
	auto scripting_data = scenario->scriptingData;

	if (input_string.find('/') != string::npos) {
		std::string point_set = input_string.substr(0, input_string.find('/'));
		std::string point = input_string.substr(input_string.find('/') + 1);

		int point_set_index = FIND_TAG_BLOCK_STRING(&scripting_data.data->pointSets,
			sizeof(cs_point_set_block),
			offsetof(cs_point_set_block, name),
			point_set);
		if (point_set_index != NONE)
		{
			cs_point_set_block *points = &scripting_data.data->pointSets.data[point_set_index];
			int point_index = FIND_TAG_BLOCK_STRING(&points->points,
				sizeof(cs_point_block),
				offsetof(cs_point_block, name),
				point);

			if (point_index != NONE)
			{
				data_store->output = (point_set_index << 16 | point_index);
				return 1;
			}
			else {
				hs_converter_error(data_store, "No such point.");
				return false;
			}
		}
		else {
			hs_converter_error(data_store, "No such point set.");
			return false;
		}
	}
	else {
		hs_converter_error(data_store, "Invalid format.");
		return false;
	}
}

#define set_hs_converter(type, func) \
	hs_convert_lookup_table[static_cast<int>(type)] = func;

void H2ToolPatches::fix_hs_converters()
{
	void **hs_convert_lookup_table = reinterpret_cast<void**>(0x009F0C88);
	set_hs_converter(hs_type::ai_behavior, hs_convert_ai_behaviour);
	set_hs_converter(hs_type::conversation, hs_convert_conversation);
	set_hs_converter(hs_type::ai_orders, hs_convert_ai_orders);
	set_hs_converter(hs_type::ai, hs_convert_ai);
	set_hs_converter(hs_type::point_reference, hs_convert_point_ref);

	// hacky workaround, lets the user directly input the ID it's meant to generate.
	hs_type passthrough_types[] = {
		hs_type::style,           hs_type::hud_message,
		hs_type::navpoint,        hs_type::ai_command_list
	};

	for (auto i : passthrough_types)
		set_hs_converter(i, hs_convert_internal_id_passthrough);
}

#undef set_hs_converter

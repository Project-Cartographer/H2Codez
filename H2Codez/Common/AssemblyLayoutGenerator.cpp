#include "AssemblyLayoutGenerator.h"
#include "TagDefinitions.h"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include "util/string_util.h"
#include "util/numerical.h"

using boost::property_tree::ptree;
using boost::property_tree::write_xml;
using boost::property_tree::xml_writer_settings;
using boost::property_tree::xml_writer_make_settings;
using namespace TagDefinitions;
using numerical::radix;

std::map<int, tag_def*> tag_definition_mapping;

int field_sizes[tag_field::count];
bool fields_without_zero_size[tag_field::count];

static size_t get_static_element_size(tag_field::field_type type)
{
	switch (type)
	{
	case tag_field::block:
	case tag_field::tag_reference:
	case tag_field::data:
		return 0x8;
	case tag_field::word_flags:
	case tag_field::word_block_flags:
	case tag_field::short_integer:
	case tag_field::short_block_index1:
	case tag_field::short_block_index2:
	case tag_field::generic_enum:
		return sizeof(WORD);
	case tag_field::byte_block_flags:
	case tag_field::byte_flags:
	case tag_field::char_block_index1:
	case tag_field::char_block_index2: // unused
	case tag_field::char_integer:
	case tag_field::char_enum:
		return sizeof(BYTE);
	case tag_field::real:
	case tag_field::real_fraction:
	case tag_field::angle:
		return sizeof(float);
	case tag_field::string:
	case tag_field::vertex_buffer:
		return 0x20  * sizeof(char);
	case tag_field::long_string:
		return 0x100 * sizeof(char);
	case tag_field::string_id:
	case tag_field::long_integer:
	case tag_field::long_enum:
	case tag_field::long_flags:
	case tag_field::long_block_index1:
	case tag_field::long_block_index2: // unused
	case tag_field::long_block_flags: // unused
	case tag_field::tag:
	case tag_field::rgb_color:
	case tag_field::argb_color:
		return sizeof(int);
	case tag_field::real_rgb_color:
		return sizeof(colour_rgb);
	case tag_field::real_argb_color:
		return sizeof(colour_rgba);
	default:
		return 0;
	}
}

size_t DumpBlock(ptree &parent_tree, tag_block_defintions *block, size_t start_offset = 0);

size_t DumpFields(ptree &parent_tree, tag_field *_fields, size_t start_offset = 0, std::string name_suffix = "")
{
	size_t element_offset = start_offset;
	
	for (auto fields = _fields; fields->type != tag_field::terminator; fields++)
	{
		// skip adding the element to parent tree
		bool skip_field = false;
		bool visible = true;
		ptree field_tree;
		std::string element_name = "undefined";
		std::string field_name = str_trim(fields->name.get_string(), " ^)(*#");
		size_t field_size = get_static_element_size(fields->type);

		field_tree.add("<xmlattr>.name", str_trim(field_name + name_suffix));
		field_tree.add("<xmlattr>.offset", numerical::to_string(element_offset, radix::hexadecimal));

		auto add_element = [&](std::string name, std::string element_name, size_t element_size, bool increment_size = true)
		{
			ptree &element = parent_tree.add(element_name, "");
			element.add("<xmlattr>.name", str_trim(name + name_suffix));
			element.add("<xmlattr>.offset", numerical::to_string(element_offset, radix::hexadecimal));
			element.add("<xmlattr>.visible", true);
			if (increment_size)
				element_offset += element_size;
		};

		auto add_int16 = [&](std::string name, bool increment_size = true)
		{
			add_element(name, "int16", 2, increment_size);
		};

		auto add_int32 = [&](std::string name, bool increment_size = true)
		{
			add_element(name, "int32", 4, increment_size);
		};

		auto add_float32 = [&](std::string name, bool increment_size = true)
		{
			add_element(name, "float32", 4, increment_size);
		};

		auto add_range16 = [&](std::string name, bool increment_size = true)
		{
			add_element(name, "range16", 2 * 2, increment_size);
		};

		auto add_rangef = [&](std::string name, bool increment_size = true)
		{
			add_element(name, "rangef", 4 * 2, increment_size);
		};

		auto add_ranged = [&](std::string name, bool increment_size = true)
		{
			add_element(name, "ranged", 4 * 2, increment_size);
		};

		auto add_point16 = [&](std::string name, bool increment_size = true)
		{
			add_element(name, "point16", 2 * 2, increment_size);
		};

		auto add_quaternion = [&](std::string name, bool increment_size = true)
		{
			add_element(name, "quaternion", 4 * 4, increment_size);
		};

		auto add_rect16 = [&](std::string name, bool increment_size = true)
		{
			add_element(name, "rect16", 2 * 4, increment_size);
		};

		auto add_enum_contents = [&](std::string element_name, std::string value_att_name)
		{
			tag_enum_def * def = reinterpret_cast<tag_enum_def*>(fields->defintion);
			for (size_t i = 0; i < def->count; i++)
			{
				ptree &enum_option = field_tree.add(element_name, "");
				enum_option.add("<xmlattr>.name", str_trim(def->names[i].get_string(), " ^)(*#"));
				enum_option.add("<xmlattr>." + value_att_name, numerical::to_string(i, radix::hexadecimal));
			}
		};

		auto add_degree = [&](std::string name, bool is_3d, bool increment_size = true)
		{
			if (is_3d) {
				add_element(name, "degree3", 4 * 3, increment_size);
			}
			else {
				add_element(name, "degree2", 4 * 2, increment_size);
			}
		};

		auto add_point = [&](std::string name, bool is_3d, bool increment_size = true)
		{
			if (is_3d) {
				add_element(name, "point3", 4 * 3, increment_size);
			}
			else {
				add_element(name, "point2", 4 * 2, increment_size);
			}
		};

		auto add_vector = [&](std::string name, bool is_3d, bool increment_size = true)
		{
			if (is_3d) {
				add_element(name, "vector3", 4 * 3, increment_size);
			} else {
				add_element(name, "vector2", 4 * 2, increment_size);
			}
		};

		switch (fields->type) {
		case tag_field::block:
		{
			element_name = "tagblock";
			auto entry_size = DumpBlock(field_tree, reinterpret_cast<tag_block_defintions*>(fields->defintion));
			field_tree.add("<xmlattr>.entrySize", numerical::to_string(entry_size, radix::hexadecimal));
			break;
		}
		case tag_field::struct_:
		{
			ptree &element = parent_tree.add("comment", "");
			field_tree.add("<xmlattr>.title", str_trim(field_name));
			field_tree.add("<xmlattr>.visible", true);

			auto def = reinterpret_cast<tag_struct_defintion*>(fields->defintion);
			element_offset = DumpBlock(parent_tree, def->block, element_offset);
			skip_field = true;
			break;
		}
		case tag_field::data:
		{
			element_name = "dataref";
			auto def = reinterpret_cast<data_ref_def*>(fields->defintion);
			if (def->flags == 2)
				field_tree.add("<xmlattr>.format", "asciiz");
			break;
		}
		case tag_field::pad:
		case tag_field::skip:
			visible = false;
			LOG_CHECK(fields->name.is_empty());
			field_tree.add("<xmlattr>.pad_name", tag_field_type_names[fields->type]);
			field_size = reinterpret_cast<size_t>(fields->defintion);
			break;
		case tag_field::string:
		case tag_field::long_string:
			element_name = "ascii";
			field_tree.add("<xmlattr>.length", numerical::to_string(field_size, radix::hexadecimal));
			break;
		case tag_field::string_id:
			element_name = "stringId";
			break;
		case tag_field::old_string_id:
			add_element(field_name + " (old string id)", "stringId", 4, true);
			skip_field = true;
			break;
		case tag_field::point_2d:
			add_point16(field_name);
			skip_field = true;
			break;
		case tag_field::real_point_2d:
			add_point(field_name, false);
			skip_field = true;
			break;
		case tag_field::real_point_3d:
			add_point(field_name, true);
			skip_field = true;
			break;
		case tag_field::real_vector_2d:
			add_vector(field_name, false);
			skip_field = true;
			break;
		case tag_field::real_vector_3d:
			add_vector(field_name, true);
			skip_field = true;
			break;
		case tag_field::real_quaternion:
			add_quaternion(field_name);
			skip_field = true;
			break;
		case tag_field::real_plane_2d:
			add_vector(field_name + " [normal]", false);
			add_float32(field_name + " [distance]");
			skip_field = true;
			break;
		case tag_field::real_plane_3d:
			add_vector(field_name + " (normal)", true);
			add_float32(field_name + " (distance)");
			skip_field = true;
			break;
		case tag_field::angle:
			element_name = "degree";
			break;
		case tag_field::real_euler_angles_2d:
			add_degree(field_name, false);
			skip_field = true;
			break;
		case tag_field::real_euler_angles_3d:
			add_degree(field_name, true);
			skip_field = true;
			break;
		case tag_field::real_ahsv_color:
			add_float32(field_name + " (alpha)");
			add_float32(field_name + " (hue)");
			add_float32(field_name + " (saturation)");
			add_float32(field_name + " (value)");
			skip_field = true;
			break;
		case tag_field::real_rgb_color:
			element_name = "colorf";
			field_tree.add("<xmlattr>.format", "rgb");
			field_tree.add("<xmlattr>.alpha", "false");
			break;
		case tag_field::real_argb_color:
			element_name = "colorf";
			field_tree.add("<xmlattr>.format", "argb");
			field_tree.add("<xmlattr>.alpha", "true");
			break;
		case tag_field::rgb_color:
			element_name = "color32";
			field_tree.add("<xmlattr>.format", "rgb");
			field_tree.add("<xmlattr>.alpha", "false");
			break;
		case tag_field::argb_color:
			element_name = "color32";
			field_tree.add("<xmlattr>.format", "argb");
			field_tree.add("<xmlattr>.alpha", "true");
			break;
		case tag_field::real_bounds:
		case tag_field::real_fraction_bounds:
			add_rangef(field_name);
			skip_field = true;
			break;
		case tag_field::angle_bounds:
			add_ranged(field_name);
			skip_field = true;
			break;
		case tag_field::short_bounds:
			add_range16(field_name);
			skip_field = true;
			break;
		case tag_field::real:
		case tag_field::real_fraction:
			element_name = "float32";
			break;
		case tag_field::char_integer:
		case tag_field::char_block_index1: // temp
		case tag_field::char_block_index2: // temp/unused
			element_name = "int8";
			break;
		case tag_field::short_integer:
		case tag_field::short_block_index1:
		case tag_field::short_block_index2:
		case tag_field::word_block_flags:
			element_name = "int16";
			break;	
		case tag_field::long_integer:
		case tag_field::long_block_index1:
			element_name = "int32";
			break;
		case tag_field::tag: // not really supported by assembly
			element_name = "uint32";
			visible = false;
			break;
		case tag_field::char_enum:
			add_enum_contents("option", "value");
			element_name = "enum8";
			break;
		case tag_field::generic_enum: 
			add_enum_contents("option", "value");
			element_name = "enum16";
			break;
		case tag_field::long_enum:
			add_enum_contents("option", "value");
			element_name = "enum32";
			break;
		case tag_field::tag_reference:
			element_name = "tagRef";
			break;
		case tag_field::byte_flags:
			element_name = "bitfield8";
			add_enum_contents("bit", "index");
			break;
		case tag_field::word_flags:
			element_name = "bitfield16";
			add_enum_contents("bit", "index");
			break;
		case tag_field::long_flags:
			element_name = "bitfield32";
			add_enum_contents("bit", "index");
			break;
		case tag_field::rectangle_2d:
			add_rect16(field_name);
			skip_field = true;
			break;
		case tag_field::vertex_buffer:
			element_name = "raw";
			field_tree.add("<xmlattr>.size", numerical::to_string(field_size, radix::hexadecimal));
			break;
		case tag_field::explanation:
			if (field_name.empty())
				skip_field = true;
			element_name = "comment";
			field_tree.add("<xmlattr>.title", field_name);
			break;
		case tag_field::custom:
			visible = false;
			if (fields->group_tag.is_set() && LOG_CHECK(fields->group_tag.is_printable())) {
				element_name = "comment";
				field_tree.add("<xmlattr>.title", +" custom(" + fields->group_tag.as_string() + ")");
			} else {
				skip_field = true;
			}
			break;
		case tag_field::useless_pad: // pretty sure this isn't used
			skip_field = true;
			break;
		case tag_field::array_start:
		{
			size_t count = reinterpret_cast<size_t>(fields->defintion);
			for (size_t i = 0; i < count; i++)
			{
				element_offset = DumpFields(parent_tree, fields + 1, element_offset, "[" + std::to_string(i) + "]");
			}
			while (fields->type != tag_field::array_end && fields->type != tag_field::terminator)
				fields++;
			LOG_CHECK(fields->type == tag_field::array_end);
			skip_field = true;
			break;
		}
		case tag_field::array_end: // this case should only happen if we are called from array_start
			return element_offset;
		default:
			LOG_FUNC("Unhandled field type: %s", tag_field_type_names[fields->type]);
			break;
		}

		if (field_size == 0)
			fields_without_zero_size[fields->type] = true;
		field_sizes[fields->type] = field_size;
		field_tree.add("<xmlattr>.visible", visible);
		element_offset += field_size;
		if (!skip_field)
			parent_tree.add_child(element_name, field_tree);
	}

	return element_offset;
}

size_t DumpBlock(ptree &parent_tree, tag_block_defintions *block, size_t start_offset)
{
	LOG_FUNC("processing '%s'", block->name);
	return DumpFields(parent_tree, block->latest->fields, start_offset);	
}

size_t DumpTag(ptree &parent_tree, tag_def *def)
{
	size_t block_size = 0;
	if (def->parent_group_tag.is_set())
		block_size += DumpTag(parent_tree, tag_definition_mapping[def->parent_group_tag.as_int()]);
	return DumpBlock(parent_tree, def->block_defs, block_size);;
}

void DumpPlugin(std::string folder, tag_def *def)
{
	ptree tree;
	ptree &plugin_tree = tree.add("plugin", "");
	plugin_tree.add("<xmlattr>.game", "Halo2");

	ptree &revisions = plugin_tree.add("revisions", "");
	ptree &rev = revisions.add("revision", "Autogenerated by h2codez");
	rev.add("<xmlattr>.author", "IC");
	rev.add("<xmlattr>.version", 1);


	size_t block_size = DumpTag(plugin_tree, def);
	plugin_tree.add("<xmlattr>.baseSize", numerical::to_string(block_size, radix::hexadecimal));

	write_xml(
		folder + str_trim(sanitize_filename(def->group_tag.as_string())) + ".xml",
		tree,
		std::locale(),
		xml_writer_settings<std::string>('\t', 1)
	);
}

void AssemblyLayoutGenerator::DumpAllTags(std::string folder)
{
	auto tags = TagDefinitions::get_tag_definitions();
	auto tag_count = TagDefinitions::get_tag_count();
	for (size_t i = 0; i < tag_count; i++)
	{
		tag_def *definition = tags[i];
		tag_definition_mapping[definition->group_tag.as_int()] = definition;
	}

	for (size_t i = 0; i < tag_count; i++)
	{
		DumpPlugin(folder, tags[i]);
	}

	for (size_t i = 0; i < tag_field::count; i++)
	{
		LOG_FUNC("Field %s size: %d", tag_field_type_names[i], field_sizes[i]);
	}
	for (size_t i = 0; i < tag_field::count; i++)
	{
		if (fields_without_zero_size[i])
			LOG_FUNC("Field is zero size %s", tag_field_type_names[i]);
	}
}

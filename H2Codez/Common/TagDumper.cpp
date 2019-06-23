#include "TagDumper.h"
#include "TagInterface.h"
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

static size_t get_static_element_size(tag_field::field_type type)
{
	switch (type)
	{
		case tag_field::block:
			return sizeof(tag_block_ref);
		case tag_field::tag_reference:
			return sizeof(tag_reference);
		case tag_field::data:
			return sizeof(byte_ref);
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
			return 0x20 * sizeof(char);
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
		case tag_field::real_hsv_color:
		case tag_field::real_rgb_color:
			return sizeof(colour_rgb);
		case tag_field::real_ahsv_color:
		case tag_field::real_argb_color:
			return sizeof(colour_rgba);
		case tag_field::real_point_3d:
			return sizeof(real_point3d);
		case tag_field::real_point_2d:
			return sizeof(real_point2d);
		case tag_field::point_2d:
			return sizeof(point2d);
		case tag_field::rectangle_2d:
			return sizeof(rect2d);
		case tag_field::real_vector_2d:
			return sizeof(real_vector2d);
		case tag_field::real_vector_3d:
			return sizeof(real_vector3d);
		case tag_field::real_quaternion:
			return sizeof(real_quaternion);
		case tag_field::real_euler_angles_2d:
			return sizeof(real_euler_angles2d);
		case tag_field::real_euler_angles_3d:
			return sizeof(real_euler_angles3d);
		case tag_field::real_plane_2d:
			return sizeof(real_plane2d);
		case tag_field::real_plane_3d:
			return sizeof(real_plane3d);
		case tag_field::angle_bounds:
			return sizeof(angle_bounds);

		case tag_field::real_bounds:
		case tag_field::real_fraction_bounds:
			return sizeof(real_bounds);

		case tag_field::short_bounds:
			return sizeof(short_bounds);

		case tag_field::explanation:
		case tag_field::custom:
		case tag_field::pad:
		case tag_field::skip:
		case tag_field::useless_pad:
		case tag_field::array_start:
			return 0;
		default:
			LOG_FUNC("%s", tag_field_type_names[type]);
			return 0;
	}
}

static void dump_tag_block(tag_block_ref *block, ptree &tree);
static size_t dump_tag_element(tag_field *fields, char *data, ptree &tree);
static size_t dump_tag_field(tag_field **fields_pointer, char *data, ptree &tree);
static size_t dump_tag_array(tag_field *fields, char *data, ptree &tree);

static size_t dump_tag_field(tag_field **fields_pointer, char *data, ptree &tree)
{
	ASSERT_CHECK(data != nullptr);
	ASSERT_CHECK(fields_pointer != nullptr);
	ASSERT_CHECK(*fields_pointer != nullptr);
	ASSERT_CHECK((*fields_pointer)->type != tag_field::array_end);

	tag_field *fields = *fields_pointer;
	size_t size_change = get_static_element_size(fields->type);
	std::string name = str_trim(fields->name.get_string(), " ^)(*#");
	name = name.substr(0, name.find_first_of('#'));

	if (fields->type == tag_field::block) {
		ptree &block_tree = tree.add("block", "");
		block_tree.add("<xmlattr>.name", name);
		tag_block_ref *block = reinterpret_cast<tag_block_ref*>(data);
		dump_tag_block(block, block_tree);
	}
	else if (fields->type == tag_field::struct_) {
		ptree &struct_tree = tree.add("struct", "");
		struct_tree.add("<xmlattr>.name", name);
		auto def = reinterpret_cast<tag_struct_defintion*>(fields->defintion);
		size_change = dump_tag_element(def->block->latest->fields, data, struct_tree);
	}
	else if (fields->type == tag_field::array_start) {
		ptree &array_tree = tree.add("array", "");
		array_tree.add("<xmlattr>.name", name);
		size_t count = reinterpret_cast<size_t>(fields->defintion);
		for (size_t i = 0; i < count; i++)
		{
			size_change += dump_tag_array(fields, &data[size_change], array_tree);
		}
		while (fields->type != tag_field::array_end && fields->type != tag_field::terminator)
			fields++;
		LOG_CHECK(fields->type == tag_field::array_end);
		*fields_pointer = fields;
	}
	else if (fields->type == tag_field::tag_reference) {
		tag_reference *tag_ref = reinterpret_cast<tag_reference *>(data);
		std::string tag_path = tag_ref->tag_name ? tag_ref->tag_name : "NONE";
		ptree &ref_tree = tree.add("tag_reference", tag_path);
		ref_tree.add("<xmlattr>.name", name);
	}
	else {
		std::string value;
		ptree &field_tree = tree.add("field", "");
		field_tree.add("<xmlattr>.name", name);
		field_tree.add("<xmlattr>.type", tag_field_type_names[fields->type]);
		switch (fields->type)
		{
			case tag_field::pad:
			case tag_field::skip:
			case tag_field::useless_pad:
			{
				size_change = reinterpret_cast<size_t>(fields->defintion);
				break;
			}
			case tag_field::explanation:
			case tag_field::custom:
				return 0;
		}
	}
	return size_change;
}

static size_t dump_tag_array(tag_field *fields, char *data, ptree &tree)
{
	ASSERT_CHECK(fields->type == tag_field::array_start);
	size_t offset = 0;
	for (fields++; fields->type != tag_field::array_end; fields++)
		offset += dump_tag_field(&fields, &data[offset], tree);
	return offset;
}

static size_t dump_tag_element(tag_field *fields, char *data, ptree &tree)
{
	size_t offset = 0;
	for (;fields->type != tag_field::terminator; fields++)
		offset += dump_tag_field(&fields, &data[offset], tree);
	return offset;
}

static void dump_tag_block(tag_block_ref *block, ptree &tree)
{
	tag_field *fields = block->defination->latest->fields;
	for (size_t idx = 0; idx < block->size; idx++)
	{
		ptree &element_tree = tree.add("element", "");
		char *element = reinterpret_cast<char*>(block->get_element(idx));
		dump_tag_element(fields, element, element_tree);
	}
}

bool TagDumper::dump_as_xml(datum tag, const std::string &xml_dump_name)
{
	for (size_t i = 0; i < 60; i++)
		get_static_element_size((tag_field::field_type)i);
	ptree tree;
	ptree &tag_tree = tree.add("tag", "");
	dump_tag_block(tags::get_root_block(tag), tag_tree);

	printf("Saving to %s", xml_dump_name.c_str());

	write_xml(
		xml_dump_name + ".xml",
		tree,
		std::locale(),
		xml_writer_settings<std::string>('\t', 1)
	);
	return true;
}
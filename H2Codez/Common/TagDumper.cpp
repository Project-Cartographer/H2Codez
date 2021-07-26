#include "TagDumper.h"
#include "TagInterface.h"
#include "TagDefinitions.h"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include "util/string_util.h"
#include "util/numerical.h"
#include "util/XMLWriter.h"
#include <fstream>

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
		case tag_field::old_string_id:
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

		default:
			return 0;
	}
}

static void dump_tag_block(XMLWriter& writer, tag_block_ref *block);
static size_t dump_tag_element(XMLWriter& writer, tag_field *fields, char *data);
static size_t dump_tag_field(XMLWriter& writer, tag_field **fields_pointer, char *data);
static size_t dump_tag_array(XMLWriter& writer, tag_field **fields, char *data);

static size_t dump_tag_field(XMLWriter& writer, tag_field **fields_pointer, char *data)
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
		tag_block_ref* block = reinterpret_cast<tag_block_ref*>(data);
		std::map<FastString, FastString> attributes = { {"name", name} };

		writer.write(
			"block",
			[block](XMLWriter& writer) -> bool {
				dump_tag_block(writer, block);
				return false;
			},
			attributes
		);
	}
	else if (fields->type == tag_field::struct_) {
		std::map<FastString, FastString> attributes = { {"name", name} };
		auto def = reinterpret_cast<tag_struct_defintion*>(fields->defintion);

		writer.write(
			"struct",
			[def, data, &size_change](XMLWriter& writer) -> bool {
				size_change = dump_tag_element(writer, def->block->latest->fields, data);
				return false;
			},
			attributes
		);
	}
	else if (fields->type == tag_field::array_start) {
		std::map<FastString, FastString> attributes = { {"name", name} };

		size_t count = reinterpret_cast<size_t>(fields->defintion);
		tag_field* array_fields;
		for (size_t i = 0; i < count; i++)
		{
			array_fields = fields;

			writer.write(
				"struct",
				[&array_fields, data, &size_change](XMLWriter& writer) -> bool {
					size_change += dump_tag_array(writer, &array_fields, &data[size_change]);
					return false;
				},
				attributes
			);

			
		}
		LOG_CHECK(array_fields->type == tag_field::array_end);
		*fields_pointer = array_fields;
	}
	else if (fields->type == tag_field::tag_reference) {


		tag_reference *tag_ref = reinterpret_cast<tag_reference *>(data);
		std::string tag_path = tag_ref->tag_name ? tag_ref->tag_name : "NONE";

		std::map<FastString, FastString> attributes = { {"name", name}, {"type", tag_ref->tag_type.as_string()} };

		writer.write_self_closing("tag_reference", tag_path, attributes);
	}
	else {
		std::string value;
		std::map<FastString, FastString> attributes;
		attributes["type"] = tag_field_type_names[fields->type];
		attributes["name"] = name;

		auto write_bounds = [&](const std::string &lower, const std::string upper)
		{
			value = "[" + lower + ":" + upper + "]";
		};

		struct _2_floats
		{
			float f1;
			float f2;
		};

		struct _3_floats
		{
			float f1;
			float f2;
			float f3;
		};

		struct _4_floats
		{
			float f1;
			float f2;
			float f3;
			float f4;
		};

		auto write_2_floats = [&](_2_floats *data)
		{
			ASSERT_CHECK(data != nullptr);
			value = "{ " + std::to_string(data->f1) + ", " + std::to_string(data->f2) + " }";
		};

		auto write_3_floats = [&](_3_floats *data)
		{
			ASSERT_CHECK(data != nullptr);
			value = "{ " + std::to_string(data->f1) + ", " + std::to_string(data->f2) + ", " + std::to_string(data->f3) + " }";
		};

		auto write_4_floats = [&](_4_floats *data)
		{
			ASSERT_CHECK(data != nullptr);
			value = "{ " + std::to_string(data->f1) + ", " + std::to_string(data->f2) + ", " + std::to_string(data->f3) + ", " + std::to_string(data->f4) + " }";
		};
		switch (fields->type)
		{
			case tag_field::angle:
			{
				angle *angle_data = reinterpret_cast<angle*>(data);
				value = std::to_string(angle_data->as_degree());
				break;
			}
			case tag_field::angle_bounds:
			{
				angle_bounds *bounds = reinterpret_cast<angle_bounds*>(data);
				write_bounds(std::to_string(bounds->lower.as_degree()), std::to_string(bounds->upper.as_degree()));
				break;
			}
			case tag_field::real_bounds:
			case tag_field::real_fraction_bounds:
			{
				real_bounds *bounds = reinterpret_cast<real_bounds*>(data);
				write_bounds(std::to_string(bounds->lower), std::to_string(bounds->upper));
				break;
			}
			case tag_field::short_bounds:
			{
				short_bounds *bounds = reinterpret_cast<short_bounds*>(data);
				write_bounds(std::to_string(bounds->lower), std::to_string(bounds->upper));
				break;
			}
			case tag_field::string_id:
			case tag_field::old_string_id:
			{
				string_id *string = reinterpret_cast<string_id*>(data);
				value = numerical::to_string(string->get_packed(), numerical::hexadecimal, 8) 
					+ ":" + string->get_name();
				break;
			}
			case tag_field::long_integer:
			case tag_field::long_enum:
			case tag_field::long_flags:
			case tag_field::long_block_index1:
			case tag_field::long_block_index2: // unused
			case tag_field::long_block_flags: // unused
			{
				int *int_data = reinterpret_cast<int*>(data);
				value = std::to_string(*int_data);
				break;
			}
			case tag_field::rgb_color:
			case tag_field::argb_color:
			{
				std::stringstream ss;

				int color = *reinterpret_cast<int*>(data);

				ss << std::setfill('0') << "#";
				ss << std::hex << std::setw(8) << color;

				value = ss.str();
				break;
			}
			case tag_field::word_flags:
			case tag_field::word_block_flags:
			case tag_field::short_integer:
			case tag_field::short_block_index1:
			case tag_field::short_block_index2:
			case tag_field::generic_enum:
			{
				WORD *int_data = reinterpret_cast<WORD*>(data);
				value = std::to_string(*int_data);
				break;
			}

			case tag_field::byte_block_flags:
			case tag_field::byte_flags:
			case tag_field::char_block_index1:
			case tag_field::char_block_index2: // unused
			case tag_field::char_integer:
			case tag_field::char_enum:
			{
				BYTE *int_data = reinterpret_cast<BYTE*>(data);
				value = std::to_string(*int_data);
				break;
			}
			case tag_field::tag:
			{
				blam_tag *tag = reinterpret_cast<blam_tag*>(data);
				value = tag->as_string();
				break;
			}
			case tag_field::real:
			case tag_field::real_fraction:
			{
				float *float_data = reinterpret_cast<float*>(data);
				value = std::to_string(*float_data);
				break;
			}
			case tag_field::pad:
			case tag_field::skip:
			{
				size_change = reinterpret_cast<size_t>(fields->defintion);
				value = as_hex_string(data, size_change);
				break;
			}
			case tag_field::data:
			{
				byte_ref *data_ref = reinterpret_cast<byte_ref*>(data);
				attributes["size"] = std::to_string(data_ref->size);
				if (!data_ref->is_empty())
					value = as_hex_string(data_ref->address, data_ref->size);
				break;
			}
			case tag_field::vertex_buffer:
				value = as_hex_string(data, 0x20);
				break;
			case tag_field::string:
			case tag_field::long_string:
				value = std::string(data, strnlen_s(data, size_change));
				break;

			case tag_field::real_point_2d:
			case tag_field::real_vector_2d:
			{
				write_2_floats(reinterpret_cast<_2_floats*>(data));
				break;
			}
			case tag_field::real_rgb_color:
			case tag_field::real_vector_3d:
			case tag_field::real_point_3d:
			case tag_field::real_plane_2d:
			case tag_field::real_hsv_color:
			{
				write_3_floats(reinterpret_cast<_3_floats*>(data));
				break;
			}

			case tag_field::real_argb_color:
			case tag_field::real_ahsv_color:
			case tag_field::real_quaternion:
			case tag_field::real_plane_3d:
			{
				write_4_floats(reinterpret_cast<_4_floats*>(data));
				break;
			}
			case tag_field::point_2d:
			{
				auto point = reinterpret_cast<point2d*>(data);
				value = "{ " + std::to_string(point->x) + ", " + std::to_string(point->y) + " }";
				break;
			}
			case tag_field::rectangle_2d:
			{
				rect2d *rect = reinterpret_cast<rect2d*>(data);
				value = "top: " + std::to_string(rect->top) + " left: " + std::to_string(rect->left) +
					" bottom: " + std::to_string(rect->bottom) + " right: " + std::to_string(rect->right);
				break;
			}

			case tag_field::real_euler_angles_2d:
			{
				real_euler_angles2d *angle = reinterpret_cast<real_euler_angles2d*>(data);
				value = "yaw: " + std::to_string(angle->yaw.as_degree()) + 
					" pitch: " + std::to_string(angle->pitch.as_degree());
				break;
			}

			case tag_field::real_euler_angles_3d:
			{
				real_euler_angles3d *angle = reinterpret_cast<real_euler_angles3d*>(data);
				value = "yaw: " + std::to_string(angle->yaw.as_degree()) +
					" pitch: " + std::to_string(angle->yaw.as_degree()) + 
					" roll: " + std::to_string(angle->roll.as_degree());
				break;
			}

			case tag_field::explanation:
			case tag_field::custom:
			case tag_field::useless_pad:
				return 0;
			default:
				/* Unreachable (hopefully) */
				LOG_FUNC("handling for %s is missing", tag_field_type_names[fields->type]);
				break;
		}

		writer.write_self_closing("field", value, attributes);
	}
	return size_change;
}

static size_t dump_tag_array(XMLWriter& writer, tag_field **fields, char *data)
{
	ASSERT_CHECK((*fields)->type == tag_field::array_start);
	size_t offset = 0;
	for (*fields = ++(*fields); (*fields)->type != tag_field::array_end; *fields = ++(*fields))
		offset += dump_tag_field(writer, fields, &data[offset]);
	return offset;
}

static size_t dump_tag_element(XMLWriter& writer, tag_field *fields, char *data)
{
	size_t offset = 0;
	for (;fields->type != tag_field::terminator; fields++)
		offset += dump_tag_field(writer, &fields, &data[offset]);
	return offset;
}

static void dump_tag_block(XMLWriter &writer, tag_block_ref *block)
{
	tag_field *fields = block->defination->latest->fields;
	for (int32_t idx = 0; idx < block->size; idx++)
	{
		char* element = reinterpret_cast<char*>(block->get_element(idx));

		writer.write(
			"element",
			[idx](std::map<FastString, FastString>& attributes) {
				attributes["index"] = std::to_string(idx);
			},
			[element, fields](XMLWriter& writer) -> bool {
				dump_tag_element(writer, fields, element);
				return false;
			}
			);
	}
}

class TagDumperImpl : XMLWriter {
public:
	TagDumperImpl(const std::string& name) : filestream(name), XMLWriter(filestream) {};

	void dump(datum tag) {
		tag_block_ref *root = tags::get_root_block(tag);
		this->write(
			"tag",
			[root](XMLWriter& writer) -> bool {
				dump_tag_block(writer, root);
				return false;
			}
		);
	}

	std::ofstream filestream;
};

bool TagDumper::dump_as_xml(datum tag, std::string xml_dump_name)
{
	xml_dump_name += ".xml";

	printf("Saving to %s\n", xml_dump_name.c_str());

	TagDumperImpl output(xml_dump_name);
	output.dump(tag);

	return true;
}

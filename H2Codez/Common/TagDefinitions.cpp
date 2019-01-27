#include "TagDefinitions.h"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>

using boost::property_tree::ptree;
using boost::property_tree::write_xml;
using boost::property_tree::xml_writer_settings;
using boost::property_tree::xml_writer_make_settings;
using namespace TagDefinitions;

void TagDefinitions::init()
{

}

tag_def **TagDefinitions::get_tag_definitions()
{
	return reinterpret_cast<tag_def**>(SwitchAddessByMode(0, 0x98DA48, 0x901B90));
}


// generate a property tree from tag defintions
class TagDefinitions::PTreeGenerator
{
public:
	PTreeGenerator() {}
	PTreeGenerator(tag_def **_tag_definitions, size_t _tag_count) :
		tag_definitions(_tag_definitions),
		tag_count(_tag_count)
	{}

	// generates the tree, returing success
	bool generate_all()
	{
		for (size_t i = 0; i < tag_count; i++)
		{
			tag_def *definition = tag_definitions[i];
			tag_property_trees[definition->name] = generate_tag(definition);
		}
		return true;
	}

	ptree generate_tag(tag_def *tag)
	{
		ptree tree;
		ptree &tag_tree = tree.add("tag", "");
		tag_tree.add("<xmlattr>.flags", tag->flags);
		tag_tree.add("<xmlattr>.version", tag->version);
		tag_tree.add("<xmlattr>.group_tag", tag->group_tag.as_string());
		if (!tag->parent_group_tag.is_none())
			tag_tree.add("<xmlattr>.group_tag", tag->parent_group_tag.as_string());
		if (tag->default_tag_path)
			tag_tree.add("<xmlattr>.default_tag_path", tag->default_tag_path);
		generate_block(tag_tree, tag->block_defs);
		return tree;
	}

	// returns a reference to the tag name, tree mapping
	const std::map<std::string, ptree> &get_tree_map() const
	{
		return tag_property_trees;
	}

private:
	void generate_block(ptree &parent_tree, tag_block_defintions *block)
	{
		LOG_CHECK(block->latest == &block->verions[block->version_count - 1]);
		ptree &block_tree = parent_tree.add("block", "");
		block_tree.add("<xmlattr>.flags", block->flags);
		block_tree.add("<xmlattr>.name", block->name);
		block_tree.add("<xmlattr>.internal_name", block->internal_name);
		block_tree.add("<xmlattr>.max_count", block->max_count);
		block_tree.add("<xmlattr>.version_count", block->version_count);

		for (size_t i = 0; i < block->version_count; i++)
		{
			auto *set = &block->verions[i];
			if (!LOG_CHECK(set->fields == set->version.fields))
				LOG_FUNC("set name : %s", set->byteswap_name);
			ptree &version_tree = block_tree.add("set", "");
			version_tree.add("<xmlattr>.is_lastest", block->latest == set);
			version_tree.add("<xmlattr>.alignment_bit", set->alignment_bit);
			version_tree.add("<xmlattr>.toolkit_size", set->size);
			version_tree.add("<xmlattr>.version", i + 1);

			generate_fields(version_tree, set->fields);
		}

	}

	void generate_fields(ptree &parent_tree, tag_field *fields)
	{
		for (; fields->type != tag_field::terminator; fields++)
			generate_field(parent_tree, *fields);
	}

	void generate_field(ptree &parent_tree, tag_field field)
	{
		ptree &field_tree = parent_tree.add("field", "");
		field_tree.add("<xmlattr>.type", tag_field_type_names[field.type]);
		if (!field.group_tag.is_null())
			field_tree.add("<xmlattr>.group_tag", field.group_tag.as_string());
		if (!field.name.is_empty())
			field_tree.add("<xmlattr>.description", field.name.get_string());

		switch (field.type)
		{
		case tag_field::block:
			generate_block(parent_tree, reinterpret_cast<tag_block_defintions*>(field.defintion));
		case tag_field::struct_:
			//generate_struct(field_tree, reinterpret_cast<tag_struct_defintion*>(field.defintion));
		}
	}

	void generate_struct(ptree &parent_tree, tag_struct_defintion *struct_def)
	{
		ptree &struct_tree = parent_tree.add("struct", "");
		generate_block(struct_tree, struct_def->block);
	}


	std::map<std::string, ptree> tag_property_trees;
	tag_def **tag_definitions;
	size_t tag_count;
};

void TagDefinitions::dump_as_xml(std::string folder)
{
	PTreeGenerator tag_generator(get_tag_definitions(), get_tag_count());
	tag_generator.generate_all();
	for (const auto tag_tree : tag_generator.get_tree_map())
	{
		write_xml(folder + tag_tree.first + ".xml", tag_tree.second, std::locale(), xml_writer_settings<std::string>('\t', 1));
	}
}

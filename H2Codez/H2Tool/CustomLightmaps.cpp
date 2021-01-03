#include "CustomLightmaps.h"
#include "H2ToolLibrary.inl"
#include "Common/FiloInterface.h"
#include "Common/TagInterface.h"
#include "Common/Pathfinding.h"
#include "Common/tag_group_names.h"
#include "Common/TagDumper.h"
#include "Common/H2EKCommon.h"
#include "Common/RenderGeometryDumper.h"
#include "util/string_util.h"
#include "util/time.h"
#include "Tags/ScenarioStructureBSP.h"
#include "Tags/ScenarioStructureLightmap.h"
#include "Tags/ScenarioTag.h"
#include "Tags/Scenery.h"
#include "Tags/Model.h"
#include "Tags/Bitmap.h"
#include "Tags/RenderModel.h"
#include "util/Patches.h"
#include "util/process.h"
#include "util/FileSystem.h"
#include <iostream>
#include <sstream>
#include <codecvt>
#include <unordered_set>
#include <direct.h>
#include <experimental/filesystem>
#include <util/SmartHandle.h>
namespace fs = std::experimental::filesystem;

static void export_lightmap_mesh(const std::string &scenario_name, const std::string &bsp_name, const fs::path proxy_directory)
{
	tags::s_scoped_handle scenario_tag = load_tag_no_processing('scnr', scenario_name);
	if (!scenario_tag) {
		cout << "no such scenario tag!" << endl;
		return;
	}
	auto scenario = ASSERT_CHECK(tags::get_tag<scnr_tag>('scnr', scenario_tag));
	const char* bsp_tag_path = nullptr, * lightmap_tag_path = nullptr;
	int bsp_index = NONE;
	for (int i = 0; i < scenario->structureBSPs.size; i++) {
		auto bsp = ASSERT_CHECK(scenario->structureBSPs[i]);
		if (get_path_filename(bsp->structureBSP.tag_name) == bsp_name) {
			bsp_index = i;
			bsp_tag_path = bsp->structureBSP.tag_name;
			lightmap_tag_path = bsp->structureLightmap.tag_name;
		}
	}

	tags::s_scoped_handle lightmap_tag = load_tag_no_processing('ltmp', lightmap_tag_path);
	tags::s_scoped_handle bsp_tag = load_tag_no_processing('sbsp', bsp_tag_path);
	if (!lightmap_tag || !bsp_tag) {
		cout << "no such bsp or lightmap missing!" << endl;
		return;
	}
	auto lightmap = ASSERT_CHECK(tags::get_tag<scenario_structure_lightmap_block>('ltmp', lightmap_tag));
	auto sbsp = ASSERT_CHECK(tags::get_tag<scenario_structure_bsp_block>('sbsp', bsp_tag));
	auto group = ASSERT_CHECK(lightmap->lightmapGroups[0]);


	std::string mat_suffix;
	auto map_material = [&mat_suffix](const std::string& material) -> std::string { return material + mat_suffix; };

	wcout << L"dumping lightmap info to '" << proxy_directory << "'" << endl;
	std::experimental::filesystem::create_directories(proxy_directory);
	ofstream image_mapping(proxy_directory / (bsp_name + ".bitmap_mapping.txt"));
	RenderModel2COLLADA export_collada(sbsp->materials, true, map_material);

	std::string current_material;

	cout << "dumping structure..." << endl;

	for (auto i = 0; i < group->clusters.size; i++) {
		mat_suffix = "_lightmapproxy_c_" + std::to_string(i);
		auto render_info = ASSERT_CHECK(group->clusterRenderInfo[i]);
		auto cluster = ASSERT_CHECK(group->clusters[i]);
		auto cache_data = ASSERT_CHECK(cluster->cacheData[0]);

		if (render_info->bitmapIndex != NONE)
			image_mapping << "cluster_" << i << "\t" << render_info->bitmapIndex << endl;

		export_collada.AddSectionWithInstanace("cluster_" + std::to_string(i), cache_data);
	}

	cout << "dumping instances..." << endl;

	ASSERT_CHECK(sbsp->instancedGeometryInstances.size == group->instanceRenderInfo.size);
	ASSERT_CHECK(sbsp->instancedGeometriesDefinitions.size == group->poopDefinitions.size);
	for (auto i = 0; i < sbsp->instancedGeometryInstances.size; i++) {
		mat_suffix = "_lightmapproxy_i_" + std::to_string(i);
		auto render_info = ASSERT_CHECK(group->instanceRenderInfo[i]);
		auto geo_instance = ASSERT_CHECK(sbsp->instancedGeometryInstances[i]);
		auto defintion = ASSERT_CHECK(group->poopDefinitions[geo_instance->instanceDefinition]);

		stringstream instance_name;
		instance_name << "instance_" << i << "_" << geo_instance->name.get_name();

		if (render_info->bitmapIndex != NONE)
			image_mapping << instance_name.str() << "\t" << render_info->bitmapIndex << endl;

		auto section = ASSERT_CHECK(defintion->cacheData[0]);

		// todo(num0005) this shouldn't be required
		auto transform = geo_instance->transform;
		transform.inverse_rotation();
		export_collada.AddSectionWithInstanace(instance_name.str(), section, transform);
	}

	image_mapping.close();
	lightmap_tag.clear();
	bsp_tag.clear();
	mat_suffix = "";

	cout << "dumping scenery..." << endl;

	auto get_object_name = [&](const std::string& tagname, short name_index) -> std::string {
		std::string type_name = get_path_filename(tagname);
		if (name_index != NONE)
			return std::string(ASSERT_CHECK(scenario->objectNames[name_index])->name) + " " + type_name;
		else
			return type_name;
	};

	for (const auto& scenary_instance : scenario->scenery) {
		try {
			if (scenary_instance.type == NONE) {
				cout << "Invalid scenery skipped!!" << endl;
				continue;
			}
			// skip if it's for a different BSP
			if (scenary_instance.objectData.bSPPolicy != scenario_object_datum_struct_block::AlwaysPlaced
				&& scenary_instance.objectData.objectID.originBSPIndex != bsp_index)
				continue;
			if (scenary_instance.objectData.placementFlags & scenario_object_datum_struct_block::NeverPlaced) // skip unplacable
				continue;
			if (scenary_instance.objectData.placementFlags & scenario_object_datum_struct_block::NotAutomatically) // skip not placed
				continue;
			auto variant = scenary_instance.permutationData.variantName;
			auto palette = ASSERT_CHECK(scenario->sceneryPalette[scenary_instance.type]);
			tags::s_scoped_handle scenary_tag = load_tag_no_processing('scen', palette->name.tag_name);
			if (!scenary_tag)
				continue;
			auto scenary = ASSERT_CHECK(tags::get_tag<scenery_block>('scen', scenary_tag));
			if (!variant.is_valid())
				variant = scenary->object.defaultModelVariant;

			tags::s_scoped_handle model_tag = load_tag_no_processing('hlmt', scenary->object.model.tag_name);
			if (!model_tag)
				continue;
			auto model = ASSERT_CHECK(tags::get_tag<model_block>('hlmt', model_tag));

			tags::s_scoped_handle render_model_tag = load_tag_no_processing('mode', model->renderModel.tag_name);
			if (!render_model_tag)
				continue;

			auto render_model = ASSERT_CHECK(tags::get_tag<render_model_block>('mode', render_model_tag));

			auto transform = real_matrix4x3(
				real_quaternion::from_angle(scenary_instance.objectData.rotation),
				scenary_instance.objectData.position
			);

			std::string name = "scenery_" + get_object_name(palette->name.tag_name, scenary_instance.name);

			auto variant_index = 0;
			if (variant.is_valid()) {
				variant_index = model->variants.find_string_id_element(offsetof(model_variant_block, name), variant);
			}

			if (variant_index == NONE) {
				cout << "unable to find variant " << variant.get_name() << endl;
				continue;
			}

			std::vector <const global_geometry_section_struct_block*> sections;
			auto add_permutation = [&](const render_model_permutation_block* permutation) -> bool
			{
				auto section_index = permutation->l6SectionIndexhollywood;
				auto section = ASSERT_CHECK(render_model->sections[section_index]);
				auto section_data = ASSERT_CHECK(section->sectionData[0]);
				sections.push_back(&section_data->section);
				return true;
			};

			bool export_result = false;
			auto variant_info = model->variants[variant_index];
			if (variant_info) {
				for (const auto& region : variant_info->regions) {
					auto first_permutation = ASSERT_CHECK(region.permutations[0]);
					auto render_region_idx = render_model->regions.find_string_id_element(offsetof(render_model_region_block, name), region.regionName);

					auto render_region = ASSERT_CHECK(render_model->regions[render_region_idx]);
					auto render_permutation_index = render_region->permutations.find_string_id_element(offsetof(render_model_permutation_block, name), first_permutation->permutationName);

					auto permutation = ASSERT_CHECK(render_region->permutations[render_permutation_index]);
					export_result = add_permutation(permutation);
					if (!export_result)
						break;
				}
			}
			else {
				for (const auto& region : render_model->regions) {
					auto permutation = ASSERT_CHECK(region.permutations[0]);
					export_result = add_permutation(permutation);
					if (!export_result)
						break;
				}
			}
			if (export_result) {
				export_collada.ChangeMaterialsSource(render_model->materials);
				auto mesh_id = export_collada.AddMutlipleSections(name, sections);
				export_collada.AddSectionInstance(mesh_id, name, transform);
			} else {
				cout << "failed to export '" << model->renderModel.tag_name << "'" << endl;
			}

		}
		catch (const std::exception& ex) {
			cout << ex.what() << endl;
		}
	}

	cout << "Saving exported DAE" << endl;
	export_collada.Write((proxy_directory / (bsp_name + ".DAE")).string());
	cout << "Done!" << endl;
}

static void _cdecl lightmap_dump_proc(const wchar_t* argv[])
{
	auto scenario_name = wstring_to_string.to_bytes(argv[0]);
	auto bsp_name = wstring_to_string.to_bytes(argv[1]);
	auto proxy_directory = fs::path(argv[2]);
	try {
		export_lightmap_mesh(scenario_name, bsp_name, proxy_directory);
	} catch (const std::exception &ex) { 
		cout << "Exception occurred: " << ex.what() << endl;
	}
}

const s_tool_command_argument lightmap_dump_args[] =
{
	{ _tool_command_argument_type_tag_name, L"scenario" },
	{ _tool_command_argument_type_string, L"BSP" },
	{ _tool_command_argument_type_data_directory, L"proxy directory" },
};


const s_tool_command lightmap_dump
{
	L"lightmap dump",
	lightmap_dump_proc,
	lightmap_dump_args,
	ARRAYSIZE(lightmap_dump_args),
	false
};

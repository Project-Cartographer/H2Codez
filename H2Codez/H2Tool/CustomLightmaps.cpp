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

static void _cdecl lightmap_dump_proc(const wchar_t* argv[])
{
	auto scenario_name = wstring_to_string.to_bytes(argv[0]);
	auto bsp_name = wstring_to_string.to_bytes(argv[1]);
	auto proxy_directory = fs::path(argv[2]);
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

	wcout << L"dumping lightmap info to '" << argv[2] << "'" << endl;
	std::experimental::filesystem::create_directories(argv[2]);
	ofstream lightmap_export(proxy_directory / "lightmap.obj");
	ofstream image_mapping(proxy_directory / "bitmap_mapping.txt");
	RenderModel2COLLADA render_collada(sbsp->materials, true);

	std::string mat_suffix;
	constexpr bool cursematerial = true;

	auto base_vertex = 1;
	std::string current_material;
	auto set_material = [&](std::string new_mat) {
		if (cursematerial)
			new_mat += mat_suffix;
		if (new_mat == current_material)
			return;
		current_material = new_mat;
		lightmap_export << "usemtl " << new_mat << endl;
	};

	auto dump_section = [&](global_geometry_section_struct_block* section, const tag_block<global_geometry_material_block>& materials, const real_matrix4x3 transform = real_matrix4x3()) -> bool {
		lightmap_export << "## start vertexes" << endl;
		for (const auto& vertex : section->rawVertices) {

			constexpr real_matrix4x3 to_blender_coords = real_matrix4x3(
				{ 1.f, 0.f, 0.f },
				{ 0.f, 1.f, 0.f },
				{ 0.f, 0.f, 1.f }
			);

			auto position = to_blender_coords * (transform * vertex.position + transform.translation);
			auto normal = to_blender_coords * (transform * vertex.normal);

			lightmap_export << "v " << position.x << " " << position.y << " " << position.z << endl;
			lightmap_export << "vt " << vertex.primaryLightmapTexcoord.x << " " << 1 - vertex.primaryLightmapTexcoord.y << endl;
			lightmap_export << "vn " << normal.i << " " << normal.j << " " << normal.k << endl;
		}
		lightmap_export << "## end vertexes" << endl;
		for (const auto& part : section->parts) {
			std::string material_base_name = "DEFAULT";
			if (part.material != NONE) {
				auto material = ASSERT_CHECK(materials[part.material]);
				material_base_name = get_path_filename(material->shader.tag_name);
			}
			switch (part.type) {
				case global_geometry_part_block_new::NotDrawn:
					set_material("lightmapproxy_no_render");
					break;
				case global_geometry_part_block_new::OpaqueNonshadowing:
					set_material(material_base_name + "__lightmap_noshadowing");
					break;
				case global_geometry_part_block_new::OpaqueShadowOnly:
				case global_geometry_part_block_new::OpaqueShadowCasting:
				case global_geometry_part_block_new::LightmapOnly:
					set_material(material_base_name);
					break;
				case global_geometry_part_block_new::Transparent:
					set_material(material_base_name + "__lightmap_transparent");
					break;
				default:
					ASSERT_CHECK(0);
					break;
			}
			for (auto j = part.firstSubpartIndex; j < part.firstSubpartIndex + part.subpartCount; j++) {
				auto subpart = ASSERT_CHECK(section->subparts[j]);
				if (subpart->indiceslength < 3) {
					cout << "subpart " << j << " is corrupt" << endl;
					return false;
				}

				auto format_index = [&](int strip_index) -> std::string {
					auto index = *ASSERT_CHECK(section->stripIndices[strip_index]) + base_vertex;
					auto index_string = std::to_string(index);
					return " " + index_string + "/" + index_string + "/" + index_string;
				};


				if (part.flags & part.OverrideTriangleList) {
					ASSERT_CHECK(subpart->indiceslength % 3 == 0);
					for (auto i = subpart->indicesstartindex; i < subpart->indicesstartindex + subpart->indiceslength; i += 3) {
						lightmap_export << "f"
							<< format_index(i) << format_index(i + 1) << format_index(i + 2)
							<< endl;
					}
				}
				else {
					for (auto i = subpart->indicesstartindex; i < subpart->indicesstartindex + subpart->indiceslength - 2; i++) {
						lightmap_export << "f"
							<< format_index(i) << format_index(i + 1) << format_index(i + 2)
							<< endl;
					}
				}
			}
		}
		base_vertex += section->rawVertices.size;
		return true;
	};

	cout << "dumping structure..." << endl;

	for (auto i = 0; i < group->clusters.size; i++) {
		mat_suffix = "_lightmapproxy_c_" + std::to_string(i);
		auto render_info = ASSERT_CHECK(group->clusterRenderInfo[i]);
		auto cluster = ASSERT_CHECK(group->clusters[i]);
		auto cache_data = ASSERT_CHECK(cluster->cacheData[0]);

		if (render_info->bitmapIndex != NONE)
			image_mapping << "cluster_" << i << "\t" << render_info->bitmapIndex << endl;

		lightmap_export << "o cluster_" << i << endl;
		dump_section(cache_data, sbsp->materials);
		render_collada.AddSectionWithInstanace("cluster_" + std::to_string(i), cache_data);
	}

	cout << "dumping instances..." << endl;
	std::vector<RenderModel2COLLADA::SECTION_ID> instance_defs;
	size_t instance_mesh_count = 0;
	for (const auto& instance_def : group->poopDefinitions) {
		instance_defs.push_back(render_collada.AddSection("instancedef_" + std::to_string(instance_mesh_count), ASSERT_CHECK(instance_def.cacheData[0])));
	}

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

		lightmap_export << "o " << instance_name.str() << endl;
		dump_section(ASSERT_CHECK(defintion->cacheData[0]), sbsp->materials, geo_instance->transform);
		render_collada.AddSectionInstance(instance_defs[geo_instance->instanceDefinition], "instance_" + std::to_string(i), geo_instance->transform);
	}

	render_collada.Write((proxy_directory / "geo.DAE").string());

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

		lightmap_export << "o scenery_" << get_object_name(palette->name.tag_name, scenary_instance.name) << endl;

		auto variant_index = 0;
		if (variant.is_valid()) {
			variant_index = model->variants.find_string_id_element(offsetof(model_variant_block, name), variant);
		}

		if (variant_index == NONE) {
			cout << "unable to find variant " << variant.get_name() << endl;
			continue;
		}

		auto dump_render_model_permutation = [&](const render_model_permutation_block* permutation) -> bool
		{
			auto section_index = permutation->l6SectionIndexhollywood;
			auto section = ASSERT_CHECK(render_model->sections[section_index]);
			auto section_data = ASSERT_CHECK(section->sectionData[0]);
			return dump_section(&section_data->section, render_model->materials, transform);
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
				export_result = dump_render_model_permutation(permutation);
			}
		} else {
			for (const auto& region : render_model->regions) {
				auto permutation = ASSERT_CHECK(region.permutations[0]);
				export_result = dump_render_model_permutation(permutation);
			}
		}
		if (!export_result)
			cout << "failed to export '" << model->renderModel.tag_name << "'" << endl;
		
	}
	
	// TODO(num0005) make this dump more info once we move to COLLADA
	cout << "dumping lights..." << endl;
	for (const auto& light : scenario->lightFixtures) {
		auto palette = ASSERT_CHECK(scenario->lightFixturesPalette[light.type]);
		lightmap_export << "o halo_light_fixture_" << get_object_name(palette->name.tag_name, light.name) << endl;
		set_material("fake_halo_light");
		auto position = light.objectData.position;
		lightmap_export << "v " << position.x << " " << position.y << " " << position.z << endl;
		lightmap_export << "f 1 1 1" << endl;
	}

	for (const auto& light : scenario->lightVolumes) {
		auto palette = ASSERT_CHECK(scenario->lightVolumesPalette[light.type]);
		lightmap_export << "o halo_light_volume_" << get_object_name(palette->name.tag_name, light.name) << endl;
		set_material("fake_halo_light");
		auto position = light.objectData.position;
		lightmap_export << "v " << position.x << " " << position.y << " " << position.z << endl;
		lightmap_export << "f 1 1 1" << endl;
	}

	wcout << L"Dumped " << base_vertex << L" vertices" << endl;
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

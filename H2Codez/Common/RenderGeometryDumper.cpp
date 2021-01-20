#include "RenderGeometryDumper.h"
#include "util/string_util.h"

void RenderModel2COLLADA::DumpSectionToMesh(COLLADA::Mesh& mesh, const global_geometry_section_struct_block* section) {
	// skip empty sections
	if (section->rawVertices.size == 0)
		return;

	// step texcoords if needed
	if (mesh.texcoord.size() == 0) {
		mesh.texcoord.resize(_is_lightmap ? 2 : 1);
	}

	// See how many of each are already added
	auto base_normal = mesh.normal.size();
	auto base_vertex = mesh.vertices.size();
	auto base_texord = mesh.texcoord[0].size();
	// Dump all vert info for this section
	for (const auto &vert : section->rawVertices) {

		mesh.normal.push_back({ vert.normal.i, vert.normal.j, vert.normal.k });
		mesh.vertices.push_back({ vert.position.x, vert.position.y, vert.position.z });
		mesh.texcoord[0].push_back({ vert.texcoord.x, -vert.texcoord.y });

		if (_is_lightmap)
			mesh.texcoord[1].push_back({ vert.primaryLightmapTexcoord.x, 1 - vert.primaryLightmapTexcoord.y });
	}
	// Add parts from this section
	for (const auto& part : section->parts) {
		COLLADA::Mesh::Part mesh_part;
		if (part.material != NONE) {
			auto material = ASSERT_CHECK(_materials.get()[part.material]);
			std::string base_name = get_path_filename(material->shader.tag_name);
			if (_material_translator)
				base_name = _material_translator(base_name);
			mesh_part.material = base_name;
		}
		for (auto j = part.firstSubpartIndex; j < part.firstSubpartIndex + part.subpartCount; j++) {
			auto subpart = ASSERT_CHECK(section->subparts[j]);
			if (subpart->indiceslength < 3) {
				throw std::invalid_argument("Corrupt subpart!");
			}

			auto get_index = [&](int strip_index) -> size_t {
				return *ASSERT_CHECK(section->stripIndices[strip_index]);
			};

			auto add_triangle = [&](int first_strip_index) {
				COLLADA::Mesh::Triangle triangle;

				size_t list[3] = {
					get_index(first_strip_index),
					get_index(first_strip_index + 1), 
					get_index(first_strip_index + 2)
				};

				for (auto i = 0; i < ARRAYSIZE(list); i++)
					triangle.normal_list[i] = base_normal + list[i];

				for (auto i = 0; i < ARRAYSIZE(list); i++)
					triangle.texcoord_list[i] = base_texord + list[i];

				for (auto i = 0; i < ARRAYSIZE(list); i++)
					triangle.vertex_list[i] = base_vertex + list[i];

				mesh_part.triangles.push_back(triangle);
			};

			if (part.flags & part.OverrideTriangleList) {
				ASSERT_CHECK(subpart->indiceslength % 3 == 0);
				for (auto i = subpart->indicesstartindex; i < subpart->indicesstartindex + subpart->indiceslength; i += 3)
					add_triangle(i);
			}
			else {
				for (auto i = subpart->indicesstartindex; i < subpart->indicesstartindex + subpart->indiceslength - 2; i++)
					add_triangle(i);
			}
		}
		mesh.parts.push_back(mesh_part);
	}
}
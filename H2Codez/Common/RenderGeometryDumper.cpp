#include "RenderGeometryDumper.h"
#include "util/string_util.h"

RenderModel2COLLADA::SECTION_ID RenderModel2COLLADA::AddSection(const std::string& name, const global_geometry_section_struct_block* section) {
	COLLADA::Mesh mesh;
	for (const auto &vert : section->rawVertices) {
		auto &tex_source = _is_lightmap ? vert.primaryLightmapTexcoord : vert.texcoord;

		mesh.normal.push_back({ vert.normal.i, vert.normal.j, vert.normal.k });
		mesh.vertices.push_back({ vert.position.x, vert.position.y, vert.position.z });
		mesh.texcoord.push_back({ tex_source.x, tex_source.y });
	}
	for (const auto& part : section->parts) {
		COLLADA::Mesh::Part mesh_part;
		if (part.material != NONE) {
			auto material = ASSERT_CHECK(_materials[part.material]);
			mesh_part.material = get_path_filename(material->shader.tag_name);
		}
		for (auto j = part.firstSubpartIndex; j < part.firstSubpartIndex + part.subpartCount; j++) {
			auto subpart = ASSERT_CHECK(section->subparts[j]);
			if (subpart->indiceslength < 3) {
				throw std::invalid_argument("Corrupt subpart!");
			}

			auto get_index = [&](int strip_index) -> size_t {
				return *ASSERT_CHECK(section->stripIndices[strip_index]);
			};

			if (part.flags & part.OverrideTriangleList) {
				ASSERT_CHECK(subpart->indiceslength % 3 == 0);
				for (auto i = subpart->indicesstartindex; i < subpart->indicesstartindex + subpart->indiceslength; i += 3) {
					COLLADA::Mesh::Triangle triangle;
					size_t list[3] = { get_index(i), get_index(i + 1), get_index(i + 2) };
					std::copy(list, &list[ARRAYSIZE(list)], triangle.normal_list);
					std::copy(list, &list[ARRAYSIZE(list)], triangle.texcoord_list);
					std::copy(list, &list[ARRAYSIZE(list)], triangle.vertex_list);
					mesh_part.triangles.push_back(triangle);
				}
			}
			else {
				for (auto i = subpart->indicesstartindex; i < subpart->indicesstartindex + subpart->indiceslength - 2; i++) {
					COLLADA::Mesh::Triangle triangle;
					size_t list[3] = { get_index(i), get_index(i + 1), get_index(i + 2) };
					std::copy(list, &list[ARRAYSIZE(list)], triangle.normal_list);
					std::copy(list, &list[ARRAYSIZE(list)], triangle.texcoord_list);
					std::copy(list, &list[ARRAYSIZE(list)], triangle.vertex_list);
					mesh_part.triangles.push_back(triangle);
				}
			}
		}
		mesh.parts.push_back(mesh_part);
	}
	return _collada.AddMesh(name, mesh);
}
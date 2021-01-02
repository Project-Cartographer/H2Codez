#pragma once
#include "COLLADA/COLLADA.h"
#include "Tags/GlobalGeometry.h"

class RenderModel2COLLADA {
public:
	RenderModel2COLLADA(const tag_block<global_geometry_material_block>& materials, bool is_lightmap) :
		_materials(materials),
		_is_lightmap(is_lightmap),
		_scene(_collada.AddScene("Scene"))
	{
	};

	typedef COLLADA::MeshHandle SECTION_ID;

	/// <summary>
	/// Adds a BLAM geometry section to the COLLADA tree
	/// </summary>
	/// <param name="name">The name of the section</param>
	/// <param name="section">The geometry section</param>
	/// <returns>ID of the section</returns>
	SECTION_ID AddSection(const std::string &name, const global_geometry_section_struct_block* section);

	/// <summary>
	/// Adds a BLAM geometry section to the COLLADA tree and create an instance of it
	/// </summary>
	/// <param name="name">The name of the section</param>
	/// <param name="section">The geometry section</param>
	/// <param name="transform">The transform for the instance</param>
	/// <returns>ID of the section</returns>
	inline SECTION_ID AddSectionWithInstanace(const std::string& name, const global_geometry_section_struct_block* section, const real_matrix4x3& transform = real_matrix4x3()) {
		auto section_mesh = AddSection(name, section);
		AddSectionInstance(section_mesh, name, transform);
		return section_mesh;
	}

	/// <summary>
	/// Add an instance of a section
	/// </summary>
	/// <param name="section">ID of the section</param>
	/// <param name="name">Name of the instance</param>
	/// <param name="transform">transform for the instance</param>
	inline void AddSectionInstance(SECTION_ID section, const std::string& name, const real_matrix4x3& transform = real_matrix4x3()) {
		auto node = _collada.AddNode(_scene, name, transform);
		_collada.NodeAddInstanceGeo(node, name, section);
	}

	/// <summary>
	/// Write the DAE file to the file system, forwards args to COLLADA::Write
	/// </summary>
	/// <param name="path">Output path</param>
	inline void Write(const std::string& path) {
		_collada.Write(path);
	}
private:
	COLLADA _collada = COLLADA("Z_UP");
	const tag_block<global_geometry_material_block>& _materials;
	const bool _is_lightmap;
	COLLADA::SceneHandle _scene;
};

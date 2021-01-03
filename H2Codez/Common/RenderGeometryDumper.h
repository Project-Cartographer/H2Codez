#pragma once
#include "COLLADA/COLLADA.h"
#include "Tags/GlobalGeometry.h"
#include <functional>

class RenderModel2COLLADA {
public:
	typedef std::function<std::string(const std::string&)> StringMapping;

	RenderModel2COLLADA(const tag_block<global_geometry_material_block>& materials, bool is_lightmap, StringMapping material_translator = StringMapping()) : // = [](const std::string& in) -> std::string {return in; }
		_materials(materials),
		_is_lightmap(is_lightmap),
		_scene(_collada.AddScene("Scene")),
		_material_translator(material_translator)
	{
	};

	typedef COLLADA::MeshHandle MESH_ID;

	/// <summary>
	/// Adds a BLAM geometry section to the COLLADA tree
	/// </summary>
	/// <param name="name">The name of the section</param>
	/// <param name="section">The geometry section</param>
	/// <returns>ID of the mesh</returns>
	MESH_ID AddSection(const std::string& name, const global_geometry_section_struct_block* section) {
		COLLADA::Mesh mesh;
		DumpSectionToMesh(mesh, section);
		return _collada.AddMesh(name, mesh);
	}

	/// <summary>
	/// Adds BLAM geometry sections to the COLLADA tree as a single mesh
	/// </summary>
	/// <param name="name">The name of the mesh</param>
	/// <param name="sections">The geometry sections</param>
	/// <returns>ID of the mesh</returns>
	MESH_ID AddMutlipleSections(const std::string& name, const std::vector<const global_geometry_section_struct_block*> &sections) {
		COLLADA::Mesh mesh;
		for (auto section : sections)
			DumpSectionToMesh(mesh, section);
		return _collada.AddMesh(name, mesh);
	}

	/// <summary>
	/// Adds a BLAM geometry section to the COLLADA tree and create an instance of it
	/// </summary>
	/// <param name="name">The name of the section</param>
	/// <param name="section">The geometry section</param>
	/// <param name="transform">The transform for the instance</param>
	/// <returns>ID of the mesh</returns>
	inline MESH_ID AddSectionWithInstanace(const std::string& name, const global_geometry_section_struct_block* section, const real_matrix4x3& transform = real_matrix4x3()) {
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
	inline void AddSectionInstance(MESH_ID section, const std::string& name, const real_matrix4x3& transform = real_matrix4x3()) {
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

	/// <summary>
	/// Exposes the internal COLLADA object;
	/// </summary>
	/// <returns>A reference to the COLLADA</returns>
	inline COLLADA& GetCollada() {
		return _collada;
	}

	/// <summary>
	/// Switch the materials reference used when dumping future sections
	/// </summary>
	/// <param name="new_materials"></param>
	inline void ChangeMaterialsSource(const tag_block<global_geometry_material_block>& new_materials) {
		_materials = new_materials;
	}

private:
	void DumpSectionToMesh(COLLADA::Mesh& mesh, const global_geometry_section_struct_block* section);

	COLLADA _collada = COLLADA("Z_UP");
	std::reference_wrapper<const tag_block<global_geometry_material_block>> _materials;
	const bool _is_lightmap;
	COLLADA::SceneHandle _scene;
	StringMapping _material_translator;
};

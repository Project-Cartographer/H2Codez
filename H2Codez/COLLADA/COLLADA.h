#pragma once
#include <boost/property_tree/ptree.hpp>
#include <vector>
#include <unordered_map>
#include <array>
#include <iostream>
#include "Common/BlamBaseTypes.h"

using boost::property_tree::ptree;

class COLLADA {
public:

	COLLADA(const std::string up_axis = "Z_UP")
	{
		GetAsset().add("up_axis", up_axis);
	}

	/// <summary>
	/// A tuple of float values
	/// </summary>
	template <size_t count>
	struct FloatElement {
		float values[count];
	};

	enum class HandleTypes {
		Scene,
		Node,
		Mesh,
		Instance
	};
	template <HandleTypes type>
	class TreeHandle
	{
		friend class COLLADA;
	public:
		TreeHandle(TreeHandle<type>& other) :
			tree(other.tree)
		{};

		TreeHandle<type> & operator=(const TreeHandle<type>&) = default;
	private:
		TreeHandle(boost::property_tree::ptree& scene_tree) :
			tree(scene_tree)
		{};

		boost::property_tree::ptree& tree;
	};

	typedef TreeHandle<HandleTypes::Scene> SceneHandle;
	typedef TreeHandle<HandleTypes::Node> NodeHandle;
	typedef TreeHandle<HandleTypes::Mesh> MeshHandle;
	typedef TreeHandle<HandleTypes::Instance> InstanceHandle;


	struct Mesh {
		std::vector<FloatElement<3>> vertices;
		std::vector<FloatElement<3>> normal;
		std::vector<FloatElement<2>> texcoord;
		struct Triangle {
			/*
			* 3 indexes that reference values in the vertices, normal and tex-coord vectors
			* one index for each point in the triangle
			* triangle direction is not defined
			*/
			size_t vertex_list[3];
			size_t normal_list[3];
			size_t texcoord_list[3];
		};
		struct Part {
			/// <summary>
			/// The halo material that a given part uses
			/// </summary>
			std::string material;
			std::vector<Triangle> triangles;
		};
		std::vector<Part> parts;
	};

	/// <summary>
	/// Adds a mesh to the COLLADA file
	/// </summary>
	/// <param name="name">The name of the mesh</param>
	/// <param name="mesh">The mesh data</param>
	/// <returns>Handle to the added mesh</returns>
	MeshHandle AddMesh(const std::string& name, const Mesh& mesh);

	/// <summary>
	/// Write the DAE file to the file system
	/// </summary>
	/// <param name="path">Output path</param>
	void Write(const std::string &path);
	
	/// <summary>
	/// Creates a scene with a the given name
	/// </summary>
	/// <param name="name">Human readable name</param>
	/// <returns>A handle to the scene</returns>
	inline SceneHandle AddScene(const std::string &name) {
		return CreateSceneInternal(name);
	}

	NodeHandle AddNode(SceneHandle scene, const std::string& name, const real_matrix4x3& transform);

	InstanceHandle NodeAddInstanceGeo(NodeHandle node, const std::string& name, MeshHandle mesh);

private:
	static inline void SetSource(ptree& element, const ptree& source) {
		element.add("<xmlattr>.source", "#" + source.get<std::string>("<xmlattr>.id"));
	}

	static inline void SetURL(ptree& element, const ptree& source) {
		element.add("<xmlattr>.url", "#" + source.get<std::string>("<xmlattr>.id"));
	}


	/// <summary>
	/// Adds a float array to the parent tree
	/// </summary>
	/// <param name="parent">parent ptree</param>
	/// <param name="values">vector containing the float values</param>
	/// <param name="id">optional ID for the array, otherwise a new UUID is used</param>
	template <size_t n>
	static inline ptree& AddFloatArray(ptree &parent, const std::vector<FloatElement<n>> &values, std::string id = COLLADA::GetNewUUID()) {
		std::string array_string = "";
		for (const auto &element : values) {
			for (const float value : element.values)
				array_string += " " + std::to_string(value);
		}
		auto &float_array = parent.add("float_array", array_string);
		float_array.add("<xmlattr>.count", values.size() * n);
		float_array.add("<xmlattr>.id", id);
		return float_array;
	}

	template <size_t n>
	static inline ptree& AddFloatSource(boost::property_tree::ptree& parent,
			const std::vector<FloatElement<n>>& values,
			std::array<const char*, n> accessor_info,
			std::string id = COLLADA::GetNewUUID())
	{
		auto &source = parent.add("source", "");
		auto &float_array = AddFloatArray(source, values);

		auto &technique_common = source.add("technique_common", "");
		auto &accessor = technique_common.add("accessor", "");
		SetSource(accessor, float_array);
		accessor.add("<xmlattr>.count", values.size());
		accessor.add("<xmlattr>.stride", n);

		for (auto p : accessor_info) {
			auto &param = accessor.add("param", "");
			param.add("<xmlattr>.name", p);
			param.add("<xmlattr>.type", "float");
		}

		
		source.add("<xmlattr>.id", id);
		return source;
	}

	/// <summary>
	/// Generate a new UUID
	/// </summary>
	/// <returns>A string containing the UUID</returns>
	static std::string GetNewUUID();

	inline boost::property_tree::ptree& CreateSceneInternal(const std::string& name) {
		auto& scene = GetVisualScenes().add("visual_scene", "");
		scene.add("<xmlattr>.id", GetNewUUID());
		scene.add("<xmlattr>.name", name);
		return scene;
	}

	inline boost::property_tree::ptree& GetOrCreate(const std::string &name) {
		auto found = root.find(name);
		if (found != root.not_found())
			return found->second;
		return root.add(name, "");
	}

	inline boost::property_tree::ptree& GetGeometries() {
		return GetOrCreate("library_geometries");
	}

	inline boost::property_tree::ptree& GetVisualScenes() {
		return GetOrCreate("library_visual_scenes");
	}

	inline boost::property_tree::ptree& GetAsset() {
		return GetOrCreate("asset");
	}

	inline boost::property_tree::ptree& GetMaterials() {
		return GetOrCreate("library_materials");
	}
	inline boost::property_tree::ptree& GetImages() {
		return GetOrCreate("library_images");
	}

	inline std::string AddMaterial(const std::string name) {
		auto existing = materials_map.find(name);
		if (existing != materials_map.end())
			return existing->second;
		
		auto id = GetNewUUID();

		auto& lib_mats = GetMaterials();
		auto& mat = lib_mats.add("material", "");
		mat.add("<xmlattr>.id", id);
		mat.add("<xmlattr>.name", name);
		materials_map[name] = id;
		return id;
	}

	boost::property_tree::ptree root;

	std::unordered_map<std::string, std::string> materials_map;
};
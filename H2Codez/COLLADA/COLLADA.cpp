#include "COLLADA.h"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/optional.hpp>
#include "util/string_util.h"
#include "util/numerical.h"
#include "h2codez.h"

#include <rpc.h>
#pragma comment(lib, "rpcrt4.lib")

using boost::property_tree::write_xml;
using boost::property_tree::xml_writer_settings;
using boost::property_tree::xml_writer_make_settings;

COLLADA::MeshHandle COLLADA::AddMesh(const std::string& name, const Mesh& mesh_source)
{
    /*
    * Sanity checks
    */
    for (size_t i = 0; i < mesh_source.texcoord.size(); i++) {
        if (mesh_source.texcoord[i].size() <= 0)
            throw std::invalid_argument("texcoord sets can't be zero sized");
        // size already compared for the last element
        if (i + 1 >= mesh_source.texcoord.size())
            continue;
        if (mesh_source.texcoord[i].size() != mesh_source.texcoord[i + 1].size())
            throw std::invalid_argument("texcoord sets need to have the same size");
    }

    auto id = GetNewUUID();
    auto &library_geometries = GetGeometries();
    auto &geometry = library_geometries.add("geometry", "");
    geometry.add("<xmlattr>.name", name);
    geometry.add("<xmlattr>.id", id);

    auto &mesh = geometry.add("mesh", "");
    
    std::array<const char*, 3> pos_des = { "X", "Y", "Z" };
    auto &positions_source = AddFloatSource(mesh, mesh_source.vertices, pos_des);
    auto &vertices_source = mesh.add("vertices", "");
    vertices_source.add("<xmlattr>.id", GetNewUUID());
    auto &input = vertices_source.add("input", "");
    input.add("<xmlattr>.semantic", "POSITION");
    SetSource(input, positions_source);

    constexpr std::array<const char*, 3> normal_des = { "X", "Y", "Z" };

    boost::optional<ptree&> normals_source;
    if (mesh_source.normal.size() > 0)
        normals_source = AddFloatSource(mesh, mesh_source.normal, normal_des);

    constexpr std::array<const char*, 2> tex_desc = { "S", "T" };

    std::vector<std::reference_wrapper<ptree>> texcoord_sources;
    for (const auto &tex_set : mesh_source.texcoord)
        texcoord_sources.push_back(AddFloatSource(mesh, tex_set, tex_desc, "UVMap"));

    for (const auto &part : mesh_source.parts) {
        auto &triangles = mesh.add("triangles", "");
        if (!part.material.empty()) {
            triangles.add("<xmlattr>.material", AddMaterial(part.material));
        }
        triangles.add("<xmlattr>.count", part.triangles.size());

        int offset = 0;
        auto &vertex_input = triangles.add("input", "");
        SetSource(vertex_input, vertices_source);
        vertex_input.add("<xmlattr>.offset", offset++);
        vertex_input.add("<xmlattr>.semantic", "VERTEX");

        if (normals_source) {
            auto& normal_input = triangles.add("input", "");
            SetSource(normal_input, *normals_source);
            normal_input.add("<xmlattr>.offset", offset++);
            normal_input.add("<xmlattr>.semantic", "NORMAL");
        }

        int tex_offset;
        for (size_t set_index = 0; set_index < texcoord_sources.size(); set_index++) {
            if (set_index == 0)
                tex_offset = offset++;
            auto& tex_input = triangles.add("input", "");
            SetSource(tex_input, texcoord_sources[set_index]);
            tex_input.add("<xmlattr>.offset", tex_offset);
            tex_input.add("<xmlattr>.semantic", "TEXCOORD");
            tex_input.add("<xmlattr>.set", set_index);
        }

        std::string data = "";
        for (auto triangle : part.triangles) {
            for (int i = 0; i < 3; i++) {
                data += " " + std::to_string(triangle.vertex_list[i]);
                if (normals_source)
                    data += " " + std::to_string(triangle.normal_list[i]);
                if (texcoord_sources.size() > 0)
                    data += " " + std::to_string(triangle.texcoord_list[i]);
            }
        }
        triangles.add("p", data);
    }

    return geometry;
}

COLLADA::NodeHandle COLLADA::AddNode(SceneHandle scene, const std::string& name, const real_matrix4x3& transform)
{
    auto& node = scene.tree.add("node", "");
    node.add("<xmlattr>.id", GetNewUUID());
    node.add("<xmlattr>.name", name);
    node.add("<xmlattr>.type", "NODE");

    std::stringstream transform_4x4;
    transform_4x4 << transform.forward.i << " " << transform.forward.j << " " << transform.forward.k << " " << transform.translation.x << " ";
    transform_4x4 << transform.left.i    << " " << transform.left.j    << " " << transform.left.k    << " " << transform.translation.y << " ";
    transform_4x4 << transform.up.i      << " " << transform.up.j      << " " << transform.up.k      << " " << transform.translation.z;
    transform_4x4 << " 0 0 0 1";
    auto &matrix = node.add("matrix", transform_4x4.str());
    matrix.add("<xmlattr>.sid", "transform");

    return node;
}

COLLADA::InstanceHandle COLLADA::NodeAddInstanceGeo(NodeHandle node, const std::string& name, MeshHandle mesh) {
    auto& instance_geometry = node.tree.add("instance_geometry", "");
    SetURL(instance_geometry, mesh.tree);
    instance_geometry.add("<xmlattr>.name", name);
    auto &bind_material_tech = instance_geometry.add("bind_material.technique_common", "");
    for (const auto& mat : materials_map) {
        auto &instance_material = bind_material_tech.add("instance_material", "");
        instance_material.add("<xmlattr>.symbol", mat.second);
        instance_material.add("<xmlattr>.target", "#" + mat.second);

        auto &bind_vertex_input = instance_material.add("bind_vertex_input", "");
        bind_vertex_input.add("<xmlattr>.semantic", "UVMap");
        bind_vertex_input.add("<xmlattr>.input_semantic", "TEXCOORD");
        bind_vertex_input.add("<xmlattr>.input_set", "0");
    }
    return instance_geometry;
}


std::string COLLADA::GetNewUUID() {

    UUID uuid;
    UuidCreate(&uuid);
    RPC_CSTR uuid_string;
    std::string result;
    if (ASSERT_CHECK(UuidToStringA(&uuid, &uuid_string) == RPC_S_OK)) {
        result = reinterpret_cast<char*>(uuid_string);
        ASSERT_CHECK(RpcStringFreeA(&uuid_string) == RPC_S_OK);
        return result;
    }
    return "BAD_UUID";
}

void COLLADA::Write(const std::string &path)
{
    write_xml(
        path,
        root,
        std::locale(),
        xml_writer_settings<std::string>('\t', 1)
    );
}

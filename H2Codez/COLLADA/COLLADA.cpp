#include "COLLADA.h"
#include <boost/property_tree/xml_parser.hpp>
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

    std::array<const char*, 3> normal_des = { "X", "Y", "Z" };
    auto &normals_source = AddFloatSource(mesh, mesh_source.normal, normal_des);

    std::array<const char*, 2> tex_desc = { "S", "T" };
    auto &texcoord_source = AddFloatSource(mesh, mesh_source.texcoord, tex_desc, "UVMap");

    for (const auto &part : mesh_source.parts) {
        auto &triangles = mesh.add("triangles", "");
        if (!part.material.empty()) {
            triangles.add("<xmlattr>.material", AddMaterial(part.material));
        }
        triangles.add("<xmlattr>.count", part.triangles.size());

        auto &vertex_input = triangles.add("input", "");
        SetSource(vertex_input, vertices_source);
        vertex_input.add("<xmlattr>.offset", "0");
        vertex_input.add("<xmlattr>.semantic", "VERTEX");

        auto &normal_input = triangles.add("input", "");
        SetSource(normal_input, normals_source);
        normal_input.add("<xmlattr>.offset", "1");
        normal_input.add("<xmlattr>.semantic", "NORMAL");

        auto &tex_input = triangles.add("input", "");
        SetSource(tex_input, texcoord_source);
        tex_input.add("<xmlattr>.offset", "2");
        tex_input.add("<xmlattr>.semantic", "TEXCOORD");
        tex_input.add("<xmlattr>.set", "0");

        std::string data = "";
        for (auto triangle : part.triangles) {
            for (int i = 0; i < 3; i++)
                data += " " + std::to_string(triangle.vertex_list[i])
                    + " " + std::to_string(triangle.normal_list[i])
                    + " " + std::to_string(triangle.texcoord_list[i]);
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
    ptree tree;
    auto &collada_node = tree.add_child("COLLADA", root);
    collada_node.add("<xmlattr>.xmlns", "http://www.collada.org/2005/11/COLLADASchema");
    collada_node.add("<xmlattr>.version", "1.4.1");
    collada_node.add("<xmlattr>.xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");

    write_xml(
        path,
        tree,
        std::locale(),
        xml_writer_settings<std::string>('\t', 1)
    );
}

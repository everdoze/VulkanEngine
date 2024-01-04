#include "mesh_loader.hpp"

#include "core/utils/string.hpp"
#include "core/logger/logger.hpp"
#include "platform/platform.hpp"
#include "vendor/tinyobj/tinyobj.hpp"
#include "systems/resource/resources/mesh/mesh_resource.hpp"
#include "resources/geometry/geometry.hpp"

namespace Engine {

    MeshLoader::MeshLoader(u32 id, std::string type_path, std::string custom_type) : ResourceLoader(id, ResourceType::MESH, type_path, custom_type) {};
    MeshLoader::MeshLoader(u32 id, std::string type_path) : ResourceLoader(id, ResourceType::MESH, type_path) {};

    Resource* MeshLoader::Load(std::string name) {
        // std::vector<Vertex3D> vertices;

        const char* format = "%s/%s%s";

        std::vector<SupportedMeshFileType> mesh_file_types(1);
        // mesh_file_types[0] = (SupportedMeshFileType){".e3dm", MeshFileType::E3DM, true};
        mesh_file_types[0] = (SupportedMeshFileType){".obj", MeshFileType::OBJ, false};

        // File* file = nullptr;

        std::string file_path;
        MeshFileType type = MeshFileType::NOT_FOUND;

        for (u32 i = 0; i < mesh_file_types.size(); ++i) {
            file_path = StringFormat(format, type_path.c_str(), name.c_str(), mesh_file_types[i].ext.c_str());
            if (FileSystem::FileExists(file_path)) {
                // file = FileSystem::FileOpen(file_path, FileMode::READ, mesh_file_types[i].is_binary);
                return LoadOBJ(file_path, name);
            }
        }
        

        ERROR("MeshLoader::Load - no mesh file found for mesh '%s'", name.c_str());
        return nullptr;
    };  
    
    Resource* MeshLoader::LoadOBJ(const std::string& file_path, const std::string& name) {
        tinyobj::attrib_t attributes;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        std::unordered_map<Vertex3D, u32> uniqueVertices{};

        std::vector<Vertex3D> vertices;
        vertices.reserve(16384);

        std::vector<u32> indices;
        indices.reserve(16384);

        std::vector<GeometryConfig> configs;
        configs.reserve(16);

        std::string warnings;
        std::string errors;
        std::string directory = GetFullDirectoryFromPath(file_path);
        if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &warnings, &errors, file_path.c_str(), directory.c_str())) {
            ERROR("MeshLoader::LoadOBJ - failed to load '%s'", file_path.c_str());
            return nullptr;
        }

        if (warnings.size()) {
            WARN("MeshLoader::LoadOBJ - %s", warnings.c_str());
        }

        for (const auto& shape: shapes) {
            
            GeometryExtent extent{};
            u32 current_material = INVALID_ID;
            for (u32 f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
                current_material = shape.mesh.material_ids[f];
                for (u32 i = 0; i < 3; ++i) {
                    tinyobj::index_t index = shape.mesh.indices[f * 3 + i];
                    Vertex3D vertex{};
                    
                    vertex.position = {
                        attributes.vertices[3 * index.vertex_index + 0],
                        attributes.vertices[3 * index.vertex_index + 1],
                        attributes.vertices[3 * index.vertex_index + 2]
                    };

                    vertex.texcoord = {
                        attributes.texcoords[2 * index.texcoord_index + 0],
                        attributes.texcoords[2 * index.texcoord_index + 1]
                    };

                    vertex.normal = {
                        attributes.normals[3 * index.normal_index + 0],
                        attributes.normals[3 * index.normal_index + 1],
                        attributes.normals[3 * index.normal_index + 2]
                    };

                    if (vertex.position.x < extent.min_extents.x) {
                        extent.min_extents.x = vertex.position.x;
                    }

                    if (vertex.position.y < extent.min_extents.y) {
                        extent.min_extents.y = vertex.position.y;
                    }

                    if (vertex.position.z < extent.min_extents.z) {
                        extent.min_extents.z = vertex.position.z;
                    }

                    if (vertex.position.x > extent.max_extents.x) {
                        extent.max_extents.x = vertex.position.x;
                    }

                    if (vertex.position.y > extent.max_extents.y) {
                        extent.max_extents.y = vertex.position.y;
                    }

                    if (vertex.position.z > extent.max_extents.z) {
                        extent.max_extents.z = vertex.position.z;
                    }

                    if (uniqueVertices.count(vertex) == 0) {
                        uniqueVertices[vertex] = vertices.size();
                        vertices.push_back(vertex);
                    }

                    indices.push_back(uniqueVertices[vertex]);
                }

            }
            
            for (u32 i = 0; i < 3; ++i) {
                extent.center[i] = (extent.min_extents[i] + extent.max_extents[i]) / 2.0f;
            }

            Geometry::GenerateTangents(vertices.size(), vertices.data(), indices.size(), indices.data());

            GeometryConfig config{};

            config.vertex_size = sizeof(Vertex3D);
            config.index_size = sizeof(u32);

            u32 vert_array_size = config.vertex_size * vertices.size();
            u32 idx_array_size = config.index_size * indices.size();
            config.vertices = Platform::AMemory(vert_array_size);
            config.indices = Platform::AMemory(idx_array_size);

            Platform::CMemory(config.vertices, vertices.data(), vert_array_size);
            Platform::CMemory(config.indices, indices.data(), idx_array_size);

            config.index_count = indices.size();
            config.vertex_count = vertices.size();

            if (current_material != INVALID_ID) {
                config.material_name = materials[current_material].name;
            }
            
            config.name = shape.name;

            config.extent = extent;
            
            configs.push_back(config);

            vertices.clear();
            indices.clear();
            uniqueVertices.clear();
        }

        return new MeshResource(id, name, file_path, configs);
    //     std::vector<glm::vec3> positions;
    //     positions.reserve(16384);

    //     std::vector<glm::vec3> normals;
    //     normals.reserve(16384);

    //     std::vector<glm::vec2> tex_coords;
    //     tex_coords.reserve(16384);

    //     std::vector<MeshGroup> groups;
    //     groups.reserve(4); 

    //     std::string line;
    //     while (true) {
    //         if (!file->ReadLine(line)) {
    //             break;
    //         }

    //         if (!line.size()) {
    //             continue;
    //         }

    //         if (line[0] == '#') {
    //             continue;
    //         }

    //         u8 first_char = line[0];
    //         switch (first_char) {
    //             case 'v': {
    //                 u8 second_char = line[1];
    //                 switch (second_char) {
    //                     case ' ': {
    //                         glm::vec3 pos;
    //                         c8 tmp[2];
    //                         sscanf(
    //                             line.c_str(), 
    //                             "%s %f %f %f", 
    //                             tmp,
    //                             &pos.x,
    //                             &pos.y,
    //                             &pos.z);
    //                         positions.push_back(pos);
    //                     } break;

    //                     case 'n': {
    //                         glm::vec3 nrm;
    //                         c8 tmp[2];
    //                         sscanf(
    //                             line.c_str(), 
    //                             "%s %f %f %f", 
    //                             tmp,
    //                             &nrm.x,
    //                             &nrm.y,
    //                             &nrm.z);
    //                         normals.push_back(nrm);
    //                     } break;

    //                     case 't': {
    //                         glm::vec2 tex_coord;
    //                         c8 tmp[2];
    //                         sscanf(
    //                             line.c_str(), 
    //                             "%s %f %f", 
    //                             tmp,
    //                             &tex_coord.x,
    //                             &tex_coord.y);
    //                         tex_coords.push_back(tex_coord);
    //                     } break;
    //                 }
    //             } break;

    //             case 's': {

    //             } break;

    //             case 'f': { 
    //                 MeshFaceData face;
    //                 c8 tmp[2];

    //                 if (normals.size() == 0 || tex_coords.size() == 0) {
    //                     sscanf(
    //                         line.c_str(), 
    //                         "%s %d %d %d",
    //                         tmp,
    //                         &face.vertices[0].position_idx,
    //                         &face.vertices[1].position_idx,
    //                         &face.vertices[2].position_idx);
    //                 } else {
    //                     sscanf(
    //                         line.c_str(), 
    //                         "%s %d/%d/%d %d/%d/%d %d/%d/%d",
    //                         tmp,
    //                         &face.vertices[0].position_idx,
    //                         &face.vertices[0].texcoord_idx,
    //                         &face.vertices[0].normal_idx,
    //                         &face.vertices[1].position_idx,
    //                         &face.vertices[1].texcoord_idx,
    //                         &face.vertices[1].normal_idx,
    //                         &face.vertices[2].position_idx,
    //                         &face.vertices[2].texcoord_idx,
    //                         &face.vertices[2].normal_idx);
    //                 }
    //                 groups
    //             } break;

    //             case 'm': {

    //             } break;

    //             default:
    //                 continue;
    //         }
    //     }

    };

}
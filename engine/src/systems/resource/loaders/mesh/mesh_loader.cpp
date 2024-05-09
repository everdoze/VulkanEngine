#include "mesh_loader.hpp"

#include "core/utils/string.hpp"
#include "core/logger/logger.hpp"
#include "platform/platform.hpp"
#include "vendor/tinyobj/tinyobj.hpp"
#include "systems/resource/resources/mesh/mesh_resource.hpp"

namespace Engine {

    MeshLoader::MeshLoader(u32 id, std::string type_path, std::string custom_type) : ResourceLoader(id, ResourceType::MESH, type_path, custom_type) {};
    MeshLoader::MeshLoader(u32 id, std::string type_path) : ResourceLoader(id, ResourceType::MESH, type_path) {};

    Resource* MeshLoader::Load(std::string name) {
        // std::vector<Vertex3D> vertices;

        const char* format = "%s/%s%s";

        std::vector<SupportedMeshFileType> mesh_file_types(2);
        mesh_file_types[0] = (SupportedMeshFileType){".e3dm", MeshFileType::E3DM, true};
        mesh_file_types[1] = (SupportedMeshFileType){".obj", MeshFileType::OBJ, false};

        // File* file = nullptr;

        std::string file_path;
        MeshFileType type = MeshFileType::NOT_FOUND;

        MeshResource* resource = nullptr;

        for (u32 i = 0; i < mesh_file_types.size(); ++i) {
            file_path = StringFormat(format, type_path.c_str(), name.c_str(), mesh_file_types[i].ext.c_str());
            if (FileSystem::FileExists(file_path)) {
                switch (mesh_file_types[i].type) {
                    case MeshFileType::E3DM: {
                        resource = LoadE3DM(file_path, name);
                    } break;

                    case MeshFileType::OBJ: {
                        resource = LoadOBJ(file_path, name);
                        std::string e3dm_path = StringFormat(format, type_path.c_str(), name.c_str(), ".e3dm");
                        WriteToE3DM(e3dm_path, name, resource->GetConfigs());
                    } break;

                    default: {
                        ERROR("MeshLoader::Load: Unknown file type.");
                    }
                }

                if (resource) {
                    break;
                }
            }
        }

        if (!resource) {
            ERROR("MeshLoader::Load - no mesh file found for mesh '%s'", name.c_str());
            return nullptr;
        }

        return resource;
    };  
    
    b8 MeshLoader::WriteToE3DM(const std::string& file_path, const std::string& name, GeometryConfigs& configs) {
        if (FileSystem::FileExists(file_path)) {
            INFO("MeshLoader::WriteToE3DM: File '%s' already exist and will be overwritten.", name.c_str());
        }

        File* file = FileSystem::FileOpen(file_path, FileMode::WRITE, true);
        if (!file) {
            ERROR("MeshLoader::WriteToE3DM: Unable to open file '%s' in write mode.", file_path.c_str());
            return false;  
        } 

        INFO("MeshLoader::WriteToE3DM: Writing to '%s'.", file_path.c_str());

        u16 version = 0x0001U;

        // Version
        file->Write(sizeof(u16), &version);

        // Name
        u32 name_length = name.size();
        file->Write(sizeof(u32), &name_length);
        file->Write(sizeof(char) * name_length, (void*)name.c_str());

        // Configs count
        u64 configs_count = configs.size();
        file->Write(sizeof(u64), &configs_count);

        // Configs
        for (auto config : configs) {
            
            // Vertex count
            file->Write(sizeof(u32), &config.vertex_count);

            // Vertex size
            file->Write(sizeof(u32), &config.vertex_size);

            // Vertices
            file->Write(config.vertex_count * config.vertex_size, config.vertices);

            // Index count
            file->Write(sizeof(u32), &config.index_count);

            // Index size
            file->Write(sizeof(u32), &config.index_size);

            // Indices
            file->Write(config.index_count * config.index_size, config.indices);

            // Config name
            u32 config_name_length = config.name.size();
            file->Write(sizeof(u32), &config_name_length);
            file->Write(sizeof(char) * config_name_length, (void*)config.name.c_str());

            // Material name
            u32 config_material_name_length = config.material_name.size();
            file->Write(sizeof(u32), &config_material_name_length);
            file->Write(sizeof(char) * config_material_name_length, (void*)config.material_name.c_str());

        }

        FileSystem::FileClose(file);

        return true;
    };

    MeshResource* MeshLoader::LoadE3DM(const std::string& file_path, const std::string& name) {
        File* file = FileSystem::FileOpen(file_path, FileMode::READ, true);
        if (!file) {
            ERROR("MeshLoader::LoadE3DM: Unable to open file '%s' in read mode.", file_path.c_str());
            return nullptr;  
        }

        // Version
        u16 version = 0;
        file->ReadBytes(sizeof(u16), &version);

        // Name
        u32 name_length = 0;
        file->ReadBytes(sizeof(u32), &name_length);

        std::string file_name;
        file_name.resize(name_length);
        file->ReadBytes(sizeof(char) * name_length, (void*)file_name.c_str());

        // Configs
        u64 configs_count = 0;
        file->ReadBytes(sizeof(u64), &configs_count);

        GeometryConfigs configs;
        configs.reserve(configs_count);

        for (u32 i = 0; i < configs_count; ++i) {

            GeometryConfig config;

            // Vertex count
            file->ReadBytes(sizeof(u32), &config.vertex_count);

            // Vertex size
            file->ReadBytes(sizeof(u32), &config.vertex_size);

            // Vertices
            u32 vert_array_size = config.vertex_count * config.vertex_size;
            config.vertices = Platform::AMemory(vert_array_size);
            file->ReadBytes(vert_array_size, config.vertices);

            // Index count
            file->ReadBytes(sizeof(u32), &config.index_count);

            // Index size
            file->ReadBytes(sizeof(u32), &config.index_size);

            // Indices
            u32 idx_array_size = config.index_count * config.index_size;
            config.indices = Platform::AMemory(idx_array_size);
            file->ReadBytes(idx_array_size, config.indices);

            // Config name
            u32 config_name_size = 0;
            file->ReadBytes(sizeof(u32), &config_name_size);

            config.name.resize(config_name_size);
            file->ReadBytes(sizeof(char) * config_name_size, (void*)config.name.c_str());

            // Config material name
            u32 config_material_name_size = 0;
            file->ReadBytes(sizeof(u32), &config_material_name_size);

            config.material_name.resize(config_material_name_size);
            file->ReadBytes(sizeof(char) * config_material_name_size, (void*)config.material_name.c_str());

            configs.push_back(config);
        }

        return new MeshResource(id, name, file_path, configs);
    };

    MeshResource* MeshLoader::LoadOBJ(const std::string& file_path, const std::string& name) {
        tinyobj::attrib_t attributes;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        std::unordered_map<Vertex3D, u32> uniqueVertices{};

        std::vector<Vertex3D> vertices;
        vertices.reserve(16384);

        std::vector<u32> indices;
        indices.reserve(16384);

        GeometryConfigs configs;
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

        for (u32 i = 0; i < shapes.size(); ++i) {
            auto& shape = shapes[i];
            GeometryExtent extent{};
            u32 current_material = shape.mesh.material_ids[0];;
            u32 index_offset = 0;
            for (u32 f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
                const u32 fv = 3;
                for (u32 i = 0; i < 3; ++i) {
                    tinyobj::index_t index = shape.mesh.indices[index_offset + i];
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
                index_offset += fv;
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
    }
}
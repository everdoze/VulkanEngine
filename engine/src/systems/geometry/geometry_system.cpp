#include "geometry_system.hpp"

#include "core/logger/logger.hpp"
#include "core/utils/string.hpp"
#include "systems/material/material_system.hpp"
#include "renderer/renderer.hpp"
#include "platform/platform.hpp"

namespace Engine {
    GeometrySystem* GeometrySystem::instance = nullptr;

    GeometrySystem::GeometrySystem() {
        CreateDefaultGeometries();
    };

    GeometrySystem::~GeometrySystem() {
        DestroyDefaultGeometries();

        for (Geometry* geometry : registered_geometries) { 
            delete geometry;
        }

        registered_geometries.clear();
    };

    b8 GeometrySystem::Initialize() {
        if (!instance) {
            instance = new GeometrySystem();
            return true;
        }
        WARN("Geometry system already initialized.");
        return true;
    };

    void GeometrySystem::Shutdown() {
        if (instance) {
            DEBUG("Shutting down GeometrySystem");
            delete instance;
        }
    };

    Geometry* GeometrySystem::AcquireGeometryById(u32 id) {
        return registered_geometries[id];
    };

    void GeometrySystem::ReleaseGeometry(u32 id) {
        Geometry* g = registered_geometries[id];
        if (!g) {
            return;
        }
        delete g;
    };

    Geometry* GeometrySystem::LoadGeometry(GeometryConfig& config) {
        return nullptr;
    };

    void GeometrySystem::CreateDefaultGeometries() {
        std::vector<Vertex3D> verts(4);

        const f32 f = 10.0f;
    
        verts[0].position.x = -0.5 * f;  // 0    3
        verts[0].position.y = -0.5 * f;  //
        verts[0].texcoord.x = 0.0f;      //
        verts[0].texcoord.y = 0.0f;      // 2    1

        verts[1].position.y = 0.5 * f;
        verts[1].position.x = 0.5 * f;
        verts[1].texcoord.x = 1.0f;
        verts[1].texcoord.y = 1.0f;

        verts[2].position.x = -0.5 * f;
        verts[2].position.y = 0.5 * f;
        verts[2].texcoord.x = 0.0f;
        verts[2].texcoord.y = 1.0f;

        verts[3].position.x = 0.5 * f;
        verts[3].position.y = -0.5 * f;
        verts[3].texcoord.x = 1.0f;
        verts[3].texcoord.y = 0.0f;

        const u32 index_count = 6;
        std::vector<u32> indices = {0, 1, 2, 0, 3, 1};

        GeometryCreateInfo create_info;
        create_info.id = 0;
        create_info.vertices = verts.data();
        create_info.vertex_count = verts.size();
        create_info.vertex_element_size = sizeof(Vertex3D);
        create_info.indices = indices.data();
        create_info.index_count = indices.size();
        create_info.index_element_size = sizeof(u32);
        create_info.material = MaterialSystem::GetInstance()->GetDefaultMaterial();
        default_geometry = RendererFrontend::GetInstance()->CreateGeometry(create_info);
        if (!default_geometry) {
            ERROR("Error occured during creating default 3d geometry.");
        }

        // std::vector<Vertex2D> ui_verts(4);
    
        // ui_verts[0].position.x = -0.5 * f;  // 0    3
        // ui_verts[0].position.y = -0.5 * f;  //
        // ui_verts[0].texcoord.x = 0.0f;      //
        // ui_verts[0].texcoord.y = 0.0f;      // 2    1

        // ui_verts[1].position.y = 0.5 * f;
        // ui_verts[1].position.x = 0.5 * f;
        // ui_verts[1].texcoord.x = 1.0f;
        // ui_verts[1].texcoord.y = 1.0f;

        // ui_verts[2].position.x = -0.5 * f;
        // ui_verts[2].position.y = 0.5 * f;
        // ui_verts[2].texcoord.x = 0.0f;
        // ui_verts[2].texcoord.y = 1.0f;

        // ui_verts[3].position.x = 0.5 * f;
        // ui_verts[3].position.y = -0.5 * f;
        // ui_verts[3].texcoord.x = 1.0f;
        // ui_verts[3].texcoord.y = 0.0f;

        // const u32 ui_index_count = 6;
        // std::vector<u32> ui_indices = {2, 1, 0, 3, 0, 1};

        // GeometryCreateInfo ui_create_info;
        // ui_create_info.id = 0;
        // ui_create_info.vertices = ui_verts.data();
        // ui_create_info.vertex_count = ui_verts.size();
        // ui_create_info.vertex_element_size = sizeof(Vertex2D);
        // ui_create_info.indices = ui_indices.data();
        // ui_create_info.index_count = ui_indices.size();
        // ui_create_info.index_element_size = sizeof(u32);
        // ui_create_info.material = MaterialSystem::GetInstance()->GetDefaultMaterial();
        // default_ui_geometry = RendererFrontend::GetInstance()->CreateGeometry(ui_create_info);
        // if (!default_geometry) {
        //     ERROR("Error occured during creating default 2d geometry.");
        // }

    };

    void GeometrySystem::DestroyDefaultGeometries() {
        delete default_geometry;
    };

    u32 GeometrySystem::GetNewGeometryId() {
        b8 found = false;
        for (u32 i = 0; i < registered_geometries.size(); ++i) {
            if (!registered_geometries[i]) {
                return i;
            }
        }
        if (!found) {
            return registered_geometries.size();
        }
        return INVALID_ID;
    };

    Geometry* GeometrySystem::AcquireGeometryFromConfig(GeometryConfig config, b8 auto_release) {
        GeometryCreateInfo create_info = {};
        create_info.id = GetNewGeometryId();
        create_info.vertices = config.vertices;
        create_info.vertex_count = config.vertex_count;
        create_info.vertex_element_size = config.vertex_size;
        create_info.indices = config.indices;
        create_info.index_count = config.index_count;
        create_info.index_element_size = config.index_size;
        create_info.material = MaterialSystem::GetInstance()->AcquireMaterial(config.material_name);
        if (!create_info.material) {
            WARN("Unable to acquire material '%s' for geometry '%s'. Swapping to default.", config.material_name.c_str(), config.name.c_str());
            create_info.material = MaterialSystem::GetInstance()->GetDefaultMaterial();
        }
        create_info.name = config.name;

        Geometry* g = RendererFrontend::GetInstance()->CreateGeometry(create_info);
        if (!g) {
            ERROR("Error occured during creating geometry '%s'.", config.name.c_str());
        }

        if (g->GetInternalId() == registered_geometries.size()) {
            registered_geometries.push_back(g);
        } else {
            registered_geometries[g->GetInternalId()] = g;
        }
        
        return g;
    };

    Geometry* GeometrySystem::GetDefaultGeometry() {
        return default_geometry;
    };

    GeometryConfig GeometrySystem::GeneratePlainConfig(
        f32 width, f32 height, 
        u32 x_segment_count, u32 y_segment_count, 
        f32 tile_x, f32 tile_y, 
        std::string name, std::string material_name) {
        if (width == 0) {
            WARN("Width must be nonzero. Defaulting to one.");
            width = 1.0f;
        }
        if (height == 0) {
            WARN("Height must be nonzero. Defaulting to one.");
            height = 1.0f;
        }
        if (x_segment_count < 1) {
            WARN("x_segment_count must be a positive number. Defaulting to one.");
            x_segment_count = 1;
        }
        if (y_segment_count < 1) {
            WARN("y_segment_count must be a positive number. Defaulting to one.");
            y_segment_count = 1;
        }

        if (tile_x == 0) {
            WARN("tile_x must be nonzero. Defaulting to one.");
            tile_x = 1.0f;
        }
        if (tile_y == 0) {
            WARN("tile_y must be nonzero. Defaulting to one.");
            tile_y = 1.0f;
        }

        GeometryConfig config;
        const u32 v_size = sizeof(Vertex3D) * x_segment_count * y_segment_count * 4;
        const u32 i_size = sizeof(u32) * x_segment_count * y_segment_count * 6;
        config.vertices = Platform::AMemory(v_size);
        config.indices = Platform::AMemory(i_size);

        Platform::ZMemory(config.vertices, v_size);
        Platform::ZMemory(config.indices, i_size);

        config.vertex_size = sizeof(Vertex3D);
        config.vertex_count = x_segment_count * y_segment_count * 4;
        config.index_size = sizeof(u32);
        config.index_count = x_segment_count * y_segment_count * 6;

        // TODO: This generates extra vertices, but we can always deduplicate them later.
        f32 seg_width = width / x_segment_count;
        f32 seg_height = height / y_segment_count;
        f32 half_width = width * 0.5f;
        f32 half_height = height * 0.5f;
        for (u32 y = 0; y < y_segment_count; ++y) {
            for (u32 x = 0; x < x_segment_count; ++x) {
                // Generate vertices
                f32 min_x = (x * seg_width) - half_width;
                f32 min_y = (y * seg_height) - half_height;
                f32 max_x = min_x + seg_width;
                f32 max_y = min_y + seg_height;
                f32 min_uvx = (x / (f32)x_segment_count) * tile_x;
                f32 min_uvy = (y / (f32)y_segment_count) * tile_y;
                f32 max_uvx = ((x + 1) / (f32)x_segment_count) * tile_x;
                f32 max_uvy = ((y + 1) / (f32)y_segment_count) * tile_y;

                u32 v_offset = ((y * x_segment_count) + x) * 4;
                Vertex3D* v0 = &((Vertex3D*)config.vertices)[v_offset + 0];
                Vertex3D* v1 = &((Vertex3D*)config.vertices)[v_offset + 1];
                Vertex3D* v2 = &((Vertex3D*)config.vertices)[v_offset + 2];
                Vertex3D* v3 = &((Vertex3D*)config.vertices)[v_offset + 3];

                v0->position.x = min_x;
                v0->position.y = min_y;
                v0->texcoord.x = min_uvx;
                v0->texcoord.y = min_uvy;

                v1->position.x = max_x;
                v1->position.y = max_y;
                v1->texcoord.x = max_uvx;
                v1->texcoord.y = max_uvy;

                v2->position.x = min_x;
                v2->position.y = max_y;
                v2->texcoord.x = min_uvx;
                v2->texcoord.y = max_uvy;

                v3->position.x = max_x;
                v3->position.y = min_y;
                v3->texcoord.x = max_uvx;
                v3->texcoord.y = min_uvy;

                // Generate indices
                u32 i_offset = ((y * x_segment_count) + x) * 6;
                ((u32*)config.indices)[i_offset + 0] = v_offset + 0;
                ((u32*)config.indices)[i_offset + 1] = v_offset + 1;
                ((u32*)config.indices)[i_offset + 2] = v_offset + 2;
                ((u32*)config.indices)[i_offset + 3] = v_offset + 0;
                ((u32*)config.indices)[i_offset + 4] = v_offset + 3;
                ((u32*)config.indices)[i_offset + 5] = v_offset + 1;
            }
        }

        if (name.size() > 0) {
            config.name = name;
        } else {
            config.name = DEFAULT_GEOMETRY_NAME;
        }

        if (material_name.size() > 0) {
            config.material_name = material_name;
        } else {
            config.material_name = DEFAULT_MATERIAL_NAME;
        }

        return config;
    };

    GeometryConfig GeometrySystem::GenerateCubeConfig(
        f32 width, f32 height, f32 depth,
        f32 tile_x, f32 tile_y,
        std::string name, std::string material_name) {

        if (width == 0) {
            WARN("Width must be nonzero. Defaulting to one.");
            width = 1.0f;
        }
        if (height == 0) {
            WARN("Height must be nonzero. Defaulting to one.");
            height = 1.0f;
        }
        if (depth == 0) {
            WARN("Depth must be nonzero. Defaulting to one.");
            depth = 1;
        }
        if (tile_x == 0) {
            WARN("tile_x must be nonzero. Defaulting to one.");
            tile_x = 1.0f;
        }
        if (tile_y == 0) {
            WARN("tile_y must be nonzero. Defaulting to one.");
            tile_y = 1.0f;
        }

        GeometryConfig config = {};
        config.vertex_size = sizeof(Vertex3D);
        config.vertex_count = 4 * 6;  // 4 verts per side, 6 sides
        config.vertices = Platform::AMemory(sizeof(Vertex3D) * config.vertex_count);
        config.index_size = sizeof(u32);
        config.index_count = 6 * 6;  // 6 indices per side, 6 sides
        config.indices = Platform::AMemory(sizeof(u32) * config.index_count);

        f32 half_width = width * 0.5f;
        f32 half_height = height * 0.5f;
        f32 half_depth = depth * 0.5f;
        f32 min_x = -half_width;
        f32 min_y = -half_height;
        f32 min_z = -half_depth;
        f32 max_x = half_width;
        f32 max_y = half_height;
        f32 max_z = half_depth;
        f32 min_uvx = 0.0f;
        f32 min_uvy = 0.0f;
        f32 max_uvx = tile_x;
        f32 max_uvy = tile_y;

        Vertex3D verts[24];

        Platform::ZMemory(verts, sizeof(Vertex3D) * 24);

        // Front face
        verts[(0 * 4) + 0].position = (glm::vec3){min_x, min_y, max_z};
        verts[(0 * 4) + 1].position = (glm::vec3){max_x, max_y, max_z};
        verts[(0 * 4) + 2].position = (glm::vec3){min_x, max_y, max_z};
        verts[(0 * 4) + 3].position = (glm::vec3){max_x, min_y, max_z};
        verts[(0 * 4) + 0].texcoord = (glm::vec2){min_uvx, min_uvy};
        verts[(0 * 4) + 1].texcoord = (glm::vec2){max_uvx, max_uvy};
        verts[(0 * 4) + 2].texcoord = (glm::vec2){min_uvx, max_uvy};
        verts[(0 * 4) + 3].texcoord = (glm::vec2){max_uvx, min_uvy};

        // Back face
        verts[(1 * 4) + 0].position = (glm::vec3){max_x, min_y, min_z};
        verts[(1 * 4) + 1].position = (glm::vec3){min_x, max_y, min_z};
        verts[(1 * 4) + 2].position = (glm::vec3){max_x, max_y, min_z};
        verts[(1 * 4) + 3].position = (glm::vec3){min_x, min_y, min_z};
        verts[(1 * 4) + 0].texcoord = (glm::vec2){min_uvx, min_uvy};
        verts[(1 * 4) + 1].texcoord = (glm::vec2){max_uvx, max_uvy};
        verts[(1 * 4) + 2].texcoord = (glm::vec2){min_uvx, max_uvy};
        verts[(1 * 4) + 3].texcoord = (glm::vec2){max_uvx, min_uvy};

        // Left
        verts[(2 * 4) + 0].position = (glm::vec3){min_x, min_y, min_z};
        verts[(2 * 4) + 1].position = (glm::vec3){min_x, max_y, max_z};
        verts[(2 * 4) + 2].position = (glm::vec3){min_x, max_y, min_z};
        verts[(2 * 4) + 3].position = (glm::vec3){min_x, min_y, max_z};
        verts[(2 * 4) + 0].texcoord = (glm::vec2){min_uvx, min_uvy};
        verts[(2 * 4) + 1].texcoord = (glm::vec2){max_uvx, max_uvy};
        verts[(2 * 4) + 2].texcoord = (glm::vec2){min_uvx, max_uvy};
        verts[(2 * 4) + 3].texcoord = (glm::vec2){max_uvx, min_uvy};

        // Right face
        verts[(3 * 4) + 0].position = (glm::vec3){max_x, min_y, max_z};
        verts[(3 * 4) + 1].position = (glm::vec3){max_x, max_y, min_z};
        verts[(3 * 4) + 2].position = (glm::vec3){max_x, max_y, max_z};
        verts[(3 * 4) + 3].position = (glm::vec3){max_x, min_y, min_z};
        verts[(3 * 4) + 0].texcoord = (glm::vec2){min_uvx, min_uvy};
        verts[(3 * 4) + 1].texcoord = (glm::vec2){max_uvx, max_uvy};
        verts[(3 * 4) + 2].texcoord = (glm::vec2){min_uvx, max_uvy};
        verts[(3 * 4) + 3].texcoord = (glm::vec2){max_uvx, min_uvy};

        // Bottom face
        verts[(4 * 4) + 0].position = (glm::vec3){max_x, min_y, max_z};
        verts[(4 * 4) + 1].position = (glm::vec3){min_x, min_y, min_z};
        verts[(4 * 4) + 2].position = (glm::vec3){max_x, min_y, min_z};
        verts[(4 * 4) + 3].position = (glm::vec3){min_x, min_y, max_z};
        verts[(4 * 4) + 0].texcoord = (glm::vec2){min_uvx, min_uvy};
        verts[(4 * 4) + 1].texcoord = (glm::vec2){max_uvx, max_uvy};
        verts[(4 * 4) + 2].texcoord = (glm::vec2){min_uvx, max_uvy};
        verts[(4 * 4) + 3].texcoord = (glm::vec2){max_uvx, min_uvy};

        // Top face
        verts[(5 * 4) + 0].position = (glm::vec3){min_x, max_y, max_z};
        verts[(5 * 4) + 1].position = (glm::vec3){max_x, max_y, min_z};
        verts[(5 * 4) + 2].position = (glm::vec3){min_x, max_y, min_z};
        verts[(5 * 4) + 3].position = (glm::vec3){max_x, max_y, max_z};
        verts[(5 * 4) + 0].texcoord = (glm::vec2){min_uvx, min_uvy};
        verts[(5 * 4) + 1].texcoord = (glm::vec2){max_uvx, max_uvy};
        verts[(5 * 4) + 2].texcoord = (glm::vec2){min_uvx, max_uvy};
        verts[(5 * 4) + 3].texcoord = (glm::vec2){max_uvx, min_uvy};

        for (u32 i = 0; i < 6; ++i) {
            u32 v_offset = i * 4;
            u32 i_offset = i * 6;
            ((u32*)config.indices)[i_offset + 0] = v_offset + 0;
            ((u32*)config.indices)[i_offset + 1] = v_offset + 1;
            ((u32*)config.indices)[i_offset + 2] = v_offset + 2;
            ((u32*)config.indices)[i_offset + 3] = v_offset + 0;
            ((u32*)config.indices)[i_offset + 4] = v_offset + 3;
            ((u32*)config.indices)[i_offset + 5] = v_offset + 1;
        }
        

        Geometry::GenerateNormals(24, verts, 36, (u32*)config.indices);
        Geometry::GenerateTangents(24, verts, 36, (u32*)config.indices);

        Platform::CMemory(config.vertices, verts, config.vertex_size * config.vertex_count);

        if (name.size() > 0) {
            config.name = name;
        } else {
            config.name = DEFAULT_GEOMETRY_NAME;
        }

        if (material_name.size() > 0) {
            config.material_name = material_name;
        } else {
            config.material_name = DEFAULT_MATERIAL_NAME;
        }

        return config;

    };

    void GeometrySystem::DisposeConfig(GeometryConfig& config) {
        if (config.indices) {
            Platform::FMemory(config.indices);
        }
        if (config.vertices) {
            Platform::FMemory(config.vertices);
        }
    };
}
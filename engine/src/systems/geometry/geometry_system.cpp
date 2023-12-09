#include "geometry_system.hpp"

#include "core/logger/logger.hpp"
#include "core/utils/string.hpp"
#include "systems/material/material_system.hpp"
#include "renderer/renderer.hpp"
#include "platform/platform.hpp"

namespace Engine {
    GeometrySystem* GeometrySystem::instance = nullptr;

    GeometrySystem::GeometrySystem() {
        CreateDefaultGeometry();
    };

    GeometrySystem::~GeometrySystem() {
        DestroyDefaultGeometry();
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

    void GeometrySystem::CreateDefaultGeometry() {
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
        create_info.vertices = verts;
        create_info.indices = indices;
        create_info.material = MaterialSystem::GetInstance()->GetDefaultMaterial();
        default_geometry = RendererFrontend::GetInstance()->CreateGeometry(create_info);
        if (!default_geometry) {
            ERROR("Error occured during creating default geometry.");
        }
    };

    void GeometrySystem::DestroyDefaultGeometry() {
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

    Geometry* GeometrySystem::AcquireGeometryFromConfig(GeometryConfig& config, b8 auto_release) {
        GeometryCreateInfo create_info = {};
        create_info.id = GetNewGeometryId();
        create_info.vertices = config.vertices;
        create_info.indices = config.indices;
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
        // TODO: Checks
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
        config.vertices = std::vector<Vertex3D>(x_segment_count * y_segment_count * 4);
        config.indices = std::vector<u32>(x_segment_count * y_segment_count * 6);

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
                Vertex3D* v0 = &config.vertices[v_offset + 0];
                Vertex3D* v1 = &config.vertices[v_offset + 1];
                Vertex3D* v2 = &config.vertices[v_offset + 2];
                Vertex3D* v3 = &config.vertices[v_offset + 3];

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
                config.indices[i_offset + 0] = v_offset + 0;
                config.indices[i_offset + 1] = v_offset + 1;
                config.indices[i_offset + 2] = v_offset + 2;
                config.indices[i_offset + 3] = v_offset + 0;
                config.indices[i_offset + 4] = v_offset + 3;
                config.indices[i_offset + 5] = v_offset + 1;
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
}
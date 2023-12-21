#pragma once

#include "defines.hpp"

namespace Engine {

    enum class BuiltinRenderpasses {
        WORLD = 0x01,
        UI = 0x02
    };

    struct Vertex3D {
        glm::vec3 position;
        glm::vec2 texcoord;
    };

    struct Vertex2D {
        glm::vec2 position;
        glm::vec2 texcoord;
    };

    struct GeometryRenderData {
        u32 object_id;
        glm::mat4 model;
        class Geometry* geometry;
    };
    
    struct RenderPacket {
        f32 delta_time;
        std::vector<GeometryRenderData> geometries;
        std::vector<GeometryRenderData> ui_geometries;
    };
};
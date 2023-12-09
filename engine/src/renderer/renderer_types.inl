#pragma once

#include "defines.hpp"

namespace Engine {

    struct Vertex3D {
        glm::vec3 position;
        glm::vec2 texcoord;
    };

    struct GlobalUniformObject {
        glm::mat4 projection; //64 bytes
        glm::mat4 view;       //64 bytes 

        glm::mat4 m_reserved0;  //64 bytes
        glm::mat4 m_reserved1;  //64 bytes 
    };
    // Nvidia cards require uniform object size of 256 bytes so here included 2 matrices to reserve additional memory

    struct MaterialUniformObject {
        glm::vec4 diffuse_color;
        glm::vec4 v_reserved0;
        glm::vec4 v_reserved1;
        glm::vec4 v_reserved2;
    };

    struct GeometryRenderData {
        u32 object_id;
        glm::mat4 model;
        class Geometry* geometry;
    };
    
    struct RenderPacket {
        f32 delta_time;
        std::vector<GeometryRenderData> geometries;
    };
};
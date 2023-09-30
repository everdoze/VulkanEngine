#pragma once

#include "defines.hpp"
#include "resources/texture/texture.hpp"

namespace Engine {

    typedef struct Vertex3D {
        glm::vec3 position;
        glm::vec2 texcoord;
    } Vertex3D;

    typedef struct GlobalUniformObject {
        glm::mat4 projection; //64 bytes
        glm::mat4 view;       //64 bytes 

        glm::mat4 m_reserved0;  //64 bytes
        glm::mat4 m_reserved1;  //64 bytes 
    } GlobalUniformObject;
    // Nvidia cards require uniform object size of 256 bytes so here included 2 matrices to reserve additional memory

    typedef struct MaterialUniformObject {
        glm::vec4 diffuse_color;
        glm::vec4 v_reserved0;
        glm::vec4 v_reserved1;
        glm::vec4 v_reserved2;
    } MaterialUniformObject;

    typedef struct GeometryRenderData {
        u32 object_id;
        glm::mat4 model;
        Ref<Texture> textures[16];
    } GeometryRenderData;

};
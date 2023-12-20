#pragma once

#include "defines.hpp"

namespace Engine {

    struct VulkanMaterialGlobalUBO {
        glm::mat4 projection; //64 bytes
        glm::mat4 view;       //64 bytes 

        glm::mat4 m_reserved0;  //64 bytes
        glm::mat4 m_reserved1;  //64 bytes 
    };
    // Nvidia cards require uniform object size of 256 bytes so here included 2 matrices to reserve additional memory

    struct VulkanMaterialInstanceUBO {
        glm::vec4 diffuse_color;
        glm::vec4 v_reserved0;
        glm::vec4 v_reserved1;
        glm::vec4 v_reserved2;
        glm::mat4 m_reserved0;
        glm::mat4 m_reserved1;
        glm::mat4 m_reserved2;
    };

    struct VulkanUIGlobalUBO {
        glm::mat4 projection; //64 bytes
        glm::mat4 view;       //64 bytes 

        glm::mat4 m_reserved0;  //64 bytes
        glm::mat4 m_reserved1;  //64 bytes 
    };
    // Nvidia cards require uniform object size of 256 bytes so here included 2 matrices to reserve additional memory

    struct VulkanUIInstanceUBO {
        glm::vec4 diffuse_color;
        glm::vec4 v_reserved0;
        glm::vec4 v_reserved1;
        glm::vec4 v_reserved2;
        glm::mat4 m_reserved0;
        glm::mat4 m_reserved1;
        glm::mat4 m_reserved2;
    };


}
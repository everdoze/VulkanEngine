#pragma once

#include "resources/material/material.hpp"

namespace Engine {

    class VulkanMaterial: public Material {
        public:
            VulkanMaterial(MaterialCreateInfo& info) : Material(info) {};
            ~VulkanMaterial();
    };

}
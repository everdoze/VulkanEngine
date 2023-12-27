#include "material.hpp"

#include "vulkan.hpp"
#include "shaders/shader.hpp"

namespace Engine {

    VulkanMaterial::VulkanMaterial(MaterialCreateInfo& info) : Material(info) {
        vulkan_shader = static_cast<VulkanShader*>(info.shader);
    };

    VulkanMaterial::~VulkanMaterial() {
        vulkan_shader->ReleaseInstanceResources(id);
    };

}
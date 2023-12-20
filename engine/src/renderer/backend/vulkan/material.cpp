#include "material.hpp"

#include "vulkan.hpp"

namespace Engine {

    VulkanMaterial::~VulkanMaterial() {
        VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());
        backend->ReleaseMaterial(this);
    };

}
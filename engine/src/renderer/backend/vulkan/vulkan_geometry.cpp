#include "vulkan_geometry.hpp"

#include "vulkan.hpp"

namespace Engine
{
    VulkanGeometry::VulkanGeometry(GeometryCreateInfo& info, VulkanGeometryCreateInfo& vk_info) : Geometry(info) {
        vertex_count = vk_info.vertex_count;
        vertex_size = vk_info.vertex_size;
        vertex_buffer_offset = vk_info.index_buffer_offset;

        index_count = vk_info.index_count;
        index_size = vk_info.index_size;
        index_buffer_offset = vk_info.index_buffer_offset;
    };

    VulkanGeometry::~VulkanGeometry() {
        if (this->GetInternalId() != INVALID_ID) {
            VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());
            backend->FreeGeometry(this);
        }
    };
} 

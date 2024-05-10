#include "geometry.hpp"

#include "vulkan.hpp"

namespace Engine
{
    VulkanGeometry::VulkanGeometry(GeometryCreateInfo& info, VulkanGeometryCreateInfo& vk_info) : Geometry(info) {
        vertex_count = vk_info.vertex_count;
        vertex_size = vk_info.vertex_size;
        vertex_memory = vk_info.vertex_memory;

        index_count = vk_info.index_count;
        index_size = vk_info.index_size;
        index_memory = vk_info.index_memory;
    };

    VulkanGeometry::~VulkanGeometry() {
        VulkanRendererBackend::GetInstance()->FreeGeometry(this);
        this->Free();
    };

    void VulkanGeometry::Free() {
        this->index_memory->FreeBlock();
        this->vertex_memory->FreeBlock();
    };
} 

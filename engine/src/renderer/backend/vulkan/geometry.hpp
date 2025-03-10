#pragma once

#include "resources/geometry/geometry.hpp"

namespace Engine
{

    struct VulkanGeometryCreateInfo {
        u32 vertex_count;
        u32 vertex_size;
        FreelistNode* vertex_memory;
        u32 index_count;
        u32 index_size;
        FreelistNode* index_memory;
    };

    class VulkanGeometry : public Geometry {
        public:
            VulkanGeometry(GeometryCreateInfo& info, VulkanGeometryCreateInfo& vk_info);
            ~VulkanGeometry();

            u32 GetVertexCount() { return vertex_count; };
            u32 GetVertexSize() { return vertex_size; };
            u32 GetVertexBufferOffset() { return vertex_memory->GetMemoryOffset(); };

            u32 GetIndexCount() { return index_count; };
            u32 GetIndexSize() { return index_size; };
            u32 GetIndexBufferOffset() { return index_memory->GetMemoryOffset(); };

            void SetVertexCount(u32 vertex_count) { this->vertex_count = vertex_count; };
            void SetVertexSize(u32 vertex_size) { this->vertex_size = vertex_size; };

            void SetIndexCount(u32 index_count) { this->index_count = index_count; };
            void SetIndexSize(u32 index_size) { this->index_size = index_size; };

            void Free();
        protected:
            u32 vertex_count;
            u32 vertex_size;
            FreelistNode* vertex_memory;
            u32 index_count;
            u32 index_size;
            FreelistNode* index_memory;
    };

}

#pragma once

#include <vulkan/vulkan.h>
#include "defines.hpp"

namespace Engine {

    class VulkanBuffer {
        public:
            u64 total_size;
            VkBuffer handle;
            VkBufferUsageFlagBits usage;
            b8 is_locked;
            VkDeviceMemory memory;
            i32 memory_index;
            u32 memory_property_flags;

            b8 ready;

            VulkanBuffer(
                u64 size,
                VkBufferUsageFlagBits usage,
                VkMemoryPropertyFlags memory_property_flags,
                b8 bind_on_create
            );

            ~VulkanBuffer();

            b8 Resize(u64 new_size, VkQueue queue, VkCommandPool pool);

            void Bind(u64 offset);

            void* LockMemory(u64 offset, u64 size, u32 flags);
            void UnlockMemory();

            void LoadData(u64 offset, u64 size, u32 flags, const void* data);

            void CopyTo(
                VkCommandPool pool,
                VkFence fence,
                VkQueue queue,
                u64 source_offset,
                VkBuffer dest,
                u64 dest_offset,
                u64 size
            );

    };

};
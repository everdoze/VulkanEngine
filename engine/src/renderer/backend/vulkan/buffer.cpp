#include "buffer.hpp"

#include "vulkan.hpp"
#include "helpers.hpp"
#include "core/logger/logger.hpp"
#include "platform/platform.hpp"

namespace Engine {

    VulkanBuffer::VulkanBuffer(
        u64 size,
        VkBufferUsageFlagBits usage,
        VkMemoryPropertyFlags memory_property_flags,
        b8 bind_on_create,
        b8 use_freelist) {
        
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();

        this->total_size = size;
        this->usage = usage;
        this->memory_property_flags = memory_property_flags;

        VkBufferCreateInfo buffer_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        buffer_info.size = size;
        buffer_info.usage = usage;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VK_CHECK(vkCreateBuffer(
            backend->GetVulkanDevice()->logical_device,
            &buffer_info,
            backend->GetVulkanAllocator(),
            &this->handle));

        // Gather memory requirements
        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(backend->GetVulkanDevice()->logical_device, this->handle, &requirements);
        this->memory_index = backend->FindMemoryIndex(requirements.memoryTypeBits, this->memory_property_flags);
        if (this->memory_index == -1) {
            ERROR("Unable to create vulkan buffer because the required memory index was not found.");
            ready = false;
            return;
        }
        
        if (use_freelist) {
            freelist = new Freelist(total_size);
        }
        

        VkMemoryAllocateInfo allocate_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        allocate_info.allocationSize = requirements.size;
        allocate_info.memoryTypeIndex = (u32)this->memory_index;

        VkResult result = vkAllocateMemory(
            backend->GetVulkanDevice()->logical_device,
            &allocate_info,
            backend->GetVulkanAllocator(),
            &this->memory);

        if (result != VK_SUCCESS) {
            ERROR("Unable to create vulkan buffer because the required memory allocation failed. Error: %s", VulkanResultString(result, false));
            ready = false;
            return;
        }

        if (bind_on_create) {
            Bind(0);
        }

        ready = true;
    };

    VulkanBuffer::~VulkanBuffer() {
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();

        if (this->memory) {
            vkFreeMemory(
                backend->GetVulkanDevice()->logical_device,
                this->memory,
                backend->GetVulkanAllocator());
            this->memory = nullptr;
        }
        
        if (this->handle) {
            vkDestroyBuffer(
                backend->GetVulkanDevice()->logical_device,
                this->handle,
                backend->GetVulkanAllocator());
            this->handle = nullptr;
        }

        this->total_size = 0;
        this->usage = (VkBufferUsageFlagBits)0;
        this->is_locked = false;
    };

    b8 VulkanBuffer::Resize(u64 new_size, VkQueue queue, VkCommandPool pool) {
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();

        VkBufferCreateInfo buffer_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        buffer_info.size = new_size;
        buffer_info.usage = this->usage;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkBuffer new_buffer;
        VK_CHECK(vkCreateBuffer(
            backend->GetVulkanDevice()->logical_device,
            &buffer_info,
            backend->GetVulkanAllocator(),
            &new_buffer));

        // Gather memory requirements
        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(backend->GetVulkanDevice()->logical_device, new_buffer, &requirements);

        VkMemoryAllocateInfo allocate_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        allocate_info.allocationSize = requirements.size;
        allocate_info.memoryTypeIndex = (u32)this->memory_index;

        VkDeviceMemory new_memory;
        VkResult result = vkAllocateMemory(
            backend->GetVulkanDevice()->logical_device,
            &allocate_info,
            backend->GetVulkanAllocator(),
            &new_memory);

        if (result != VK_SUCCESS) {
            ERROR("Unable to resize vulkan buffer because the required memory allocation failed. Error: %s", VulkanResultString(result, false));
            return false;
        }

        VK_CHECK(vkBindBufferMemory(
            backend->GetVulkanDevice()->logical_device,
            new_buffer,
            new_memory,
            0));

        this->CopyTo(pool, nullptr, queue, 0, new_buffer, 0, this->total_size);

        vkDeviceWaitIdle(backend->GetVulkanDevice()->logical_device);

        if (this->memory) {
            vkFreeMemory(
                backend->GetVulkanDevice()->logical_device,
                this->memory,
                backend->GetVulkanAllocator());
            this->memory = nullptr;
        }

        if (this->handle) {
            vkDestroyBuffer(
                backend->GetVulkanDevice()->logical_device,
                this->handle,
                backend->GetVulkanAllocator());
            this->handle = nullptr;
        }

        this->total_size = new_size;
        this->handle = new_buffer;
        this->memory = new_memory;

        return true;
    };

    void VulkanBuffer::Bind(u64 offset) {
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();
        VK_CHECK(vkBindBufferMemory(
            backend->GetVulkanDevice()->logical_device,
            this->handle,
            this->memory,
            offset));
    };

    void* VulkanBuffer::LockMemory(u64 offset, u64 size, u32 flags) {
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();
        void* data;
        VK_CHECK(vkMapMemory(
            backend->GetVulkanDevice()->logical_device,
            this->memory, 
            offset,
            size,
            flags,
            &data));
        return data;
    };

    void VulkanBuffer::UnlockMemory() {
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();
        vkUnmapMemory(
            backend->GetVulkanDevice()->logical_device,
            this->memory);
        };

    FreelistNode* VulkanBuffer::LoadData(u64 size, u32 flags, const void* data) {
        if (!freelist) {
            return nullptr;
        }
        FreelistNode* node = freelist->AllocateBlock(size);
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();
        void* data_ptr;
        VK_CHECK(vkMapMemory(
            backend->GetVulkanDevice()->logical_device,
            this->memory, 
            node->GetMemoryOffset(),
            node->GetSize(),
            flags,
            &data_ptr));
        Platform::CpMemory(data_ptr, data, size);
        vkUnmapMemory(
            backend->GetVulkanDevice()->logical_device,
            this->memory);
        return node;
    };

    b8 VulkanBuffer::LoadData(u64 offset, u64 size, u32 flags, const void* data) {
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();
        void* data_ptr;
        VK_CHECK(vkMapMemory(
            backend->GetVulkanDevice()->logical_device,
            this->memory, 
            offset,
            size,
            flags,
            &data_ptr));
        Platform::CpMemory(data_ptr, data, size);
        vkUnmapMemory(
            backend->GetVulkanDevice()->logical_device,
            this->memory);
        return true;
    };

    FreelistNode* VulkanBuffer::Allocate(u64 size) {
        if (!freelist) {
            return nullptr;
        }
        return freelist->AllocateBlock(size);
    };

    void VulkanBuffer::CopyTo(
        VkCommandPool pool,
        VkFence fence,
        VkQueue queue,
        u64 source_offset,
        VkBuffer dest,
        u64 dest_offset,
        u64 size) {
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();
        
        vkQueueWaitIdle(queue);

        VulkanCommandBuffer command_buffer = VulkanCommandBuffer(pool, true);

        command_buffer.BeginSingleUse();

        VkBufferCopy copy_region;
        copy_region.srcOffset = source_offset;
        copy_region.dstOffset = dest_offset;
        copy_region.size = size;

        vkCmdCopyBuffer(command_buffer.handle, handle, dest, 1, &copy_region);

        command_buffer.EndSingleUse(queue);
    };

    b8 VulkanBuffer::Free(u64 offset) {
        if (!freelist) {
            return false;
        }
        if (!freelist->FreeByOffset(offset)) {
            return false;
        }
        return true;
    };

};
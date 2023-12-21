#include "swapchain.hpp"
#include "vulkan.hpp"
#include "helpers.hpp"
#include "core/logger/logger.hpp"

#include "platform/platform.hpp"

namespace Engine {
    
    VulkanImage::~VulkanImage() {
        VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());
        
        vkDeviceWaitIdle(backend->GetVulkanDevice()->logical_device);

        if (this->view) {
            vkDestroyImageView(backend->GetVulkanDevice()->logical_device, this->view, backend->GetVulkanAllocator());
            this->view = nullptr;
        }
        if (this->memory) {
            vkFreeMemory(backend->GetVulkanDevice()->logical_device, this->memory, backend->GetVulkanAllocator());
            this->memory = nullptr;
        }
        if (this->handle) {
            vkDestroyImage(backend->GetVulkanDevice()->logical_device, this->handle, backend->GetVulkanAllocator());
            this->handle = nullptr;
        }
    };

    VulkanImage::VulkanImage(
        VkImageType image_type,
        u32 width,
        u32 height,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags memory_flags,
        b32 create_view,
        VkImageAspectFlags view_aspect_flags) {
        
        VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());

        this->height = height;
        this->width = width;

        // Creation info.
        VkImageCreateInfo image_create_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.extent.width = width;
        image_create_info.extent.height = height;
        image_create_info.extent.depth = 1;  // TODO: Support configurable depth.
        image_create_info.mipLevels = 4;     // TODO: Support mip mapping
        image_create_info.arrayLayers = 1;   // TODO: Support number of layers in the image.
        image_create_info.format = format;
        image_create_info.tiling = tiling;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_create_info.usage = usage;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;          // TODO: Configurable sample count.
        image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // TODO: Configurable sharing mode.

        VK_CHECK(vkCreateImage(
            backend->GetVulkanDevice()->logical_device, 
            &image_create_info, backend->GetVulkanAllocator(), 
            &this->handle));

         // Query memory requirements.
        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(backend->GetVulkanDevice()->logical_device, this->handle, &memory_requirements);

        i32 memory_type = backend->FindMemoryIndex(memory_requirements.memoryTypeBits, memory_flags);
        if (memory_type == -1) {
            ERROR("Required memory type not found. Image not valid.");
        }

        // Allocate memory
        VkMemoryAllocateInfo memory_allocate_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        memory_allocate_info.allocationSize = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex = memory_type;
        VK_CHECK(vkAllocateMemory(backend->GetVulkanDevice()->logical_device, &memory_allocate_info, backend->GetVulkanAllocator(), &this->memory));

        // Bind the memory
        VK_CHECK(vkBindImageMemory(backend->GetVulkanDevice()->logical_device, this->handle, this->memory, 0));  // TODO: configurable memory offset.

        // Create view
        if (create_view) {
            this->view = nullptr;
            VulkanImageViewCreate(format, view_aspect_flags);
        }

    };

    void VulkanImage::VulkanImageViewCreate(
            VkFormat format,
            VkImageAspectFlags aspect_flags) {

        VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());
        
        VkImageViewCreateInfo view_create_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        view_create_info.image = this->handle;
        view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;  // TODO: Make configurable.
        view_create_info.format = format;
        view_create_info.subresourceRange.aspectMask = aspect_flags;

        // TODO: Make configurable
        view_create_info.subresourceRange.baseMipLevel = 0;
        view_create_info.subresourceRange.levelCount = 1;
        view_create_info.subresourceRange.baseArrayLayer = 0;
        view_create_info.subresourceRange.layerCount = 1;

        VK_CHECK(vkCreateImageView(
            backend->GetVulkanDevice()->logical_device, 
            &view_create_info, 
            backend->GetVulkanAllocator(), 
            &this->view));

    };

    void VulkanImage::TransitionLayout(
            VulkanCommandBuffer* command_buffer, 
            VkFormat format, 
            VkImageLayout old_layout, 
            VkImageLayout new_layout) {

        VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());
        
        VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;
        barrier.srcQueueFamilyIndex = backend->GetVulkanDevice()->graphics_queue_index;
        barrier.dstQueueFamilyIndex = backend->GetVulkanDevice()->graphics_queue_index;
        barrier.image = this->handle;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags source_stage;
        VkPipelineStageFlags dest_stage;

        if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
 
            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dest_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dest_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            FATAL("Unsupported layout transition...");
            return;
        }

        vkCmdPipelineBarrier(
            command_buffer->handle,
            source_stage, dest_stage,
            0,
            0, 0,
            0, 0,
            1, &barrier);
    };

    void VulkanImage::CopyFromBuffer(
            VkBuffer buffer,
            VulkanCommandBuffer* command_buffer) {
        
        VkBufferImageCopy region;
        Platform::ZMemory(&region, sizeof(region));

        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        
        region.imageExtent.width = this->width;
        region.imageExtent.height = this->height;
        region.imageExtent.depth = 1;
        
        vkCmdCopyBufferToImage(
            command_buffer->handle,
            buffer, this->handle,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &region);
    };

};
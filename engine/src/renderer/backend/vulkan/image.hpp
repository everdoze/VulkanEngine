#pragma once 

#include <vulkan/vulkan.h>
#include "command_buffer.hpp"
#include "defines.hpp"


namespace Engine {

    class VulkanImage {
        protected:
            b8 own_image;
            
        public:
            VkImage handle;
            VkDeviceMemory memory;
            VkImageView view;
            u32 width;
            u32 height;
            VkFormat format;

        VulkanImage(
            VkImageType image_type,
            u32 width,
            u32 height,
            VkFormat format,
            VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags memory_flags,
            b32 create_view,
            VkImageAspectFlags view_aspect_flags
        );

        VulkanImage(
            u32 width,
            u32 height,
            VkImage image,
            VkFormat format,
            b32 create_view,
            VkImageAspectFlags view_aspect_flags
        );

        ~VulkanImage();

        void VulkanImageViewCreate(
            VkFormat format,
            VkImageAspectFlags aspect_flags
        );

        void TransitionLayout(
            VulkanCommandBuffer* command_buffer, 
            VkFormat format, 
            VkImageLayout old_layout, 
            VkImageLayout new_layout
        );

        void CopyFromBuffer(
            VkBuffer buffer,
            VulkanCommandBuffer* command_buffer
        );

    };

};
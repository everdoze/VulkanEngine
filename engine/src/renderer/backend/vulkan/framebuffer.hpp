#pragma once

#include "defines.hpp"
#include <vulkan/vulkan.h>

namespace Engine {

    class VulkanFramebuffer {
        public:
            VkFramebuffer handle;
            u32 attachment_count;
            VkImageView* attachments;

            VulkanFramebuffer(
                class VulkanRenderpass* renderpass,
                u32 width,
                u32 height,
                u32 attachment_count,
                VkImageView* attachments
            );

            ~VulkanFramebuffer();

            void Regenerate(u32 width, u32 height);

        private:
            class VulkanRenderpass* renderpass;
            void VulkanCreate(u32 width, u32 height);
    };

};
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
                Ref<class VulkanRenderpass> renderpass,
                u32 width,
                u32 height,
                u32 attachment_count,
                VkImageView* attachments
            );

            ~VulkanFramebuffer();

        private:
            Ref<class VulkanRenderpass> renderpass;
    };

};
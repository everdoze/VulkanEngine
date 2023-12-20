#pragma once

#include <vulkan/vulkan.h>
#include "defines.hpp"
#include "image.hpp"
#include "framebuffer.hpp"

namespace Engine {

    class VulkanSwapchain {
        public:
            VkSurfaceFormatKHR image_format;
            u8 max_frames_in_flight;
            VkSwapchainKHR handle;
            u32 image_count;
            VkImage* images;
            VkImageView* views;

            VulkanImage* depth_attachment;

            std::vector<VulkanFramebuffer*> ui_framebuffers;
            std::vector<VulkanFramebuffer*> world_framebuffers;

            b8 ready;

            VulkanSwapchain(
                u32 width,
                u32 height);

            ~VulkanSwapchain();

            VkResult Present(
                VkQueue graphics_queue,
                VkQueue present_queue,
                VkSemaphore render_complete_semaphore,
                u32 present_image_index
            );

            void GenerateUIFramebuffers(
                class VulkanRenderpass* renderpass
            );

            void GenerateWorldFramebuffers(
                class VulkanRenderpass* renderpass
            );

            void RegenerateFramebuffers(u32 width, u32 height);

            void DestroyFramebuffers();

            VkResult AcquireNextImageIndex(u64 timeout_ns, VkSemaphore image_available_semaphore, VkFence fence, u32* out_index);
    };

};
#pragma once

#include <vulkan/vulkan.h>
#include "defines.hpp"
#include "image.hpp"
#include "framebuffer.hpp"
#include "texture.hpp"
#include "render_target.hpp"

namespace Engine {

    class VulkanSwapchain {
        public:
            VkSurfaceFormatKHR image_format;
            u8 max_frames_in_flight;
            VkSwapchainKHR handle;
            u32 image_count;
            std::vector<VulkanTexture*> render_textures;

            VulkanTexture* depth_texture;

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

            VkResult AcquireNextImageIndex(u64 timeout_ns, VkSemaphore image_available_semaphore, VkFence fence, u32* out_index);
    };

};
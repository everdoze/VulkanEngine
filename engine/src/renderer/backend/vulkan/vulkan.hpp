#pragma once

#include "renderer/renderer.hpp"
#include "device.hpp"
#include "swapchain.hpp"
#include "renderpass.hpp"
#include "fence.hpp"
#include "buffer.hpp"
#include "resources/texture/texture.hpp"
#include "shaders/material_shader.hpp"
#include "vulkan_texture.hpp"

#include <vulkan/vulkan.h>

namespace Engine {

    class VulkanRendererBackend : public RendererBackend {
        public:
            VulkanRendererBackend(RendererSetup setup) : RendererBackend(setup) {};
            ~VulkanRendererBackend() {};

            std::vector<Ref<VulkanCommandBuffer>>& GetGraphicsCommandBufers() { return graphics_command_buffers; };

            VkSurfaceKHR GetVulkanSurface() { return surface; };
            void SetVulkanSurface(VkSurfaceKHR surface);

            i32 FindMemoryIndex(u32 type_filter, u32 property_flags);

            VkInstance GetVulkanInstance() { return vulkan_instance; };

            Ref<VulkanRenderpass> GetMainRenderpass() { return main_renderpass; };
           
            VkAllocationCallbacks* GetVulkanAllocator() { return allocator; };
            Ref<Device> GetVulkanDevice() { return device; };
            Ref<VulkanSwapchain> GetVulkanSwapchain () { return swapchain; };

            Ref<Texture> CreateTexture(
                std::string name,
                u32 width,
                u32 height,
                u8 channel_count,
                u8 has_transparency,
                u8* pixels
            );

            Ref<VulkanTexture> CreateTextureInternal(
                std::string name,
                u32 width,
                u32 height,
                u8 channel_count,
                u8 has_transparency,
                u8* pixels
            );

            void DrawGeometry();

            void SetImageIndex(u32 index) { image_index = index; };
            u32 GetImageIndex() { return image_index; };

            void SetCurrentFrame(u32 index) { current_frame = index; };
            u32 GetCurrentFrame() { return current_frame; };
            void NextFrame() { current_frame = (current_frame + 1) % swapchain->max_frames_in_flight; };
            f32 GetDeltaTime() { return delta_time; };

            b8 Initialize();
            void Shutdown();
            void Resized(u16 width, u16 height);
            b8 BeginFrame(f32 delta_time);
            b8 EndFrame(f32 delta_time);
            b8 UpdateGlobalState(glm::mat4 projection, glm::mat4 view, glm::vec3 view_position, glm::vec4 ambient_colour, i32 mode);
            void UpdateObject(GeometryRenderData data);

            b8 SwapchainCreate(u16 width, u16 height);
            b8 SwapchainRecreate(u16 width, u16 height);
            b8 SwapchainFullRecreate();

            void CreateCommandBuffers();
            void DestroyCommandBuffers();

            void CreateSyncObjects();
            void DestroySyncObjects();

            b8 CreateBuffers();
            void DestroyBuffers();

            b8 RenderpassCreate();

            void UploadDataRange(VkCommandPool pool, VkFence fence, VkQueue queue, Ref<VulkanBuffer> buffer, u64 offset, u64 size, void* data);

        private:
            // Adding debugger only for debug mode
            #if defined(_DEBUG)
                VkDebugUtilsMessengerEXT debug_messenger;
            #endif

            f32 delta_time;

            b8 recreating_swapchain;

            u64 framebuffer_generation;
            u64 framebuffer_last_generation;

            std::vector<Ref<VulkanCommandBuffer>> graphics_command_buffers;
            std::vector<VkSemaphore> image_available_semaphores;
            std::vector<VkSemaphore> queue_complete_semaphores;

            u32 in_flight_fence_count;
            std::vector<Ref<VulkanFence>> in_flight_fences;
            std::vector<Ref<VulkanFence>> images_in_flight;

            Ref<VulkanBuffer> object_vertex_buffer;
            Ref<VulkanBuffer> object_index_buffer;

            u32 image_index;
            u32 current_frame;

            Ref<VulkanShader> default_shader;
            Ref<Device> device;
            Ref<VulkanSwapchain> swapchain;
            Ref<VulkanRenderpass> main_renderpass;

            VkSurfaceKHR surface;
            VkInstance vulkan_instance;
            VkAllocationCallbacks* allocator = nullptr;

            u32 obj_id;
    };

};
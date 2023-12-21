#pragma once

#include "renderer/renderer.hpp"
#include "device.hpp"
#include "swapchain.hpp"
#include "renderpass.hpp"
#include "fence.hpp"
#include "buffer.hpp"
#include "material.hpp"
#include "resources/texture/texture.hpp"
#include "shaders/material_shader.hpp"
#include "shaders/ui_shader.hpp"
#include "texture.hpp"
#include "geometry.hpp"
#include "core/utils/freelist.hpp"

#include <vulkan/vulkan.h>

namespace Engine {

    class VulkanRendererBackend : public RendererBackend {
        public:
            VulkanRendererBackend(RendererSetup setup);

            ~VulkanRendererBackend() {};

            std::vector<VulkanCommandBuffer*>& GetGraphicsCommandBufers() { return graphics_command_buffers; };

            VkSurfaceKHR GetVulkanSurface() { return surface; };
            void SetVulkanSurface(VkSurfaceKHR surface);

            i32 FindMemoryIndex(u32 type_filter, u32 property_flags);

            VkInstance GetVulkanInstance() { return vulkan_instance; };

            VulkanRenderpass* GetMainRenderpass() { return world_renderpass; };
            VulkanRenderpass* GetUIRenderpass() { return ui_renderpass; };

            VkAllocationCallbacks* GetVulkanAllocator() { return allocator; };
            VulkanDevice* GetVulkanDevice() { return device; };
            VulkanSwapchain* GetVulkanSwapchain () { return swapchain; };

            Texture* CreateTexture(TextureCreateInfo& info);
            VulkanTexture* CreateTextureInternal(TextureCreateInfo& info);

            Material* CreateMaterial(MaterialCreateInfo& info);
            Geometry* CreateGeometry(GeometryCreateInfo& info);

            void GenerateFramebuffers();
            void RegenerateFramebuffers();
            b8 BeginRenderpass(u8 renderpass_id);
            b8 EndRenderpass(u8 renderpass_id);

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
            b8 UpdateGlobalWorldState(glm::mat4 projection, glm::mat4 view, glm::vec3 view_position, glm::vec4 ambient_color, i32 mode);
            b8 UpdateGlobalUIState(glm::mat4 projection, glm::mat4 view, i32 mode);
            void DrawGeometry(GeometryRenderData data);

            b8 SwapchainCreate(u16 width, u16 height);
            b8 SwapchainRecreate(u16 width, u16 height);
            b8 SwapchainFullRecreate();

            void CreateCommandBuffers();
            void DestroyCommandBuffers();

            void CreateSyncObjects();
            void DestroySyncObjects();

            b8 CreateBuffers();
            void DestroyBuffers();

            b8 RenderpassesCreate();

            void UploadDataRange(VkCommandPool pool, VkFence fence, VkQueue queue, VulkanBuffer* buffer, u64 offset, u64 size, void* data);
            void FreeDataRange(VulkanBuffer* buffer, u64 offset, u64 size);
            void FreeGeometry(VulkanGeometry* geometry);
            void ReleaseMaterial(VulkanMaterial* material);
        private:
            // Adding debugger only for debug mode
            #if defined(_DEBUG)
                VkDebugUtilsMessengerEXT debug_messenger;
            #endif

            f32 delta_time;

            b8 recreating_swapchain;

            u64 framebuffer_generation;
            u64 framebuffer_last_generation;

            std::vector<VulkanCommandBuffer*> graphics_command_buffers;
            std::vector<VkSemaphore> image_available_semaphores;
            std::vector<VkSemaphore> queue_complete_semaphores;

            u32 in_flight_fence_count;
            std::vector<VulkanFence*> in_flight_fences;
            std::vector<VulkanFence*> images_in_flight;

            VulkanBuffer* object_vertex_buffer;
            VulkanBuffer* object_index_buffer;

            u32 image_index;
            u32 current_frame;

            VulkanMaterialShader* material_shader;
            VulkanUIShader* ui_shader;

            VulkanDevice* device;
            VulkanSwapchain* swapchain;
            VulkanRenderpass* world_renderpass;
            VulkanRenderpass* ui_renderpass;

            VkSurfaceKHR surface;
            VkInstance vulkan_instance;
            VkAllocationCallbacks* allocator;

            Freelist* vertex_buffer_freelist;
            Freelist* index_buffer_freelist;

            u32 obj_id;
    };

};
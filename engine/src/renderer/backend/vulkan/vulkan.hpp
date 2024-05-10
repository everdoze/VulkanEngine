#pragma once

#include "defines.hpp"
#include "renderer/renderer.hpp"
#include "device.hpp"
#include "swapchain.hpp"
#include "renderpass.hpp"
#include "fence.hpp"
#include "buffer.hpp"
#include "material.hpp"
#include "resources/texture/texture.hpp"
#include "texture.hpp"
#include "geometry.hpp"
#include "shaders/shader.hpp"
#include "core/utils/freelist.hpp"

#include <vulkan/vulkan.h>

#define WORLD_RENDERPASS_NAME "WorldRenderpass"
#define UI_RENDERPASS_NAME "UIRenderpass"

namespace Engine {

    class VulkanRendererBackend : public RendererBackend {
        public:
            static VulkanRendererBackend* CreateBackend(RendererSetup setup) {
                instance = new VulkanRendererBackend(setup);
                return instance;
            };

            static VulkanRendererBackend* GetInstance() { 
                return instance;
            };

            static VulkanRendererBackend* instance;

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

            void GenerateFramebuffers();
            void RegenerateFramebuffers();
            b8 BeginRenderpass(u8 renderpass_id);
            b8 EndRenderpass(u8 renderpass_id);

            void SetImageIndex(u32 index) { image_index = index; };
            u32 GetImageIndex() { return image_index; };

            void SetCurrentFrame(u32 index) { current_frame = index; };
            u32 GetCurrentFrame() { return current_frame; };
            void NextFrame() { current_frame = (current_frame + 1) % swapchain->max_frames_in_flight; };
            u32 GetFrame() { return current_frame; };
            f32 GetDeltaTime() { return delta_time; };

            b8 Initialize();
            void Shutdown();
            void Resized(u16 width, u16 height);
            b8 BeginFrame(f32 delta_time);
            b8 EndFrame(f32 delta_time);
            
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

            FreelistNode* UploadDataRange(VkCommandPool pool, VkFence fence, VkQueue queue, VulkanBuffer* buffer, u64 size, void* data);
            void FreeDataRange(VulkanBuffer* buffer, u64 offset, u64 size);

            Material* CreateMaterial(MaterialCreateInfo& info);
            Geometry* CreateGeometry(GeometryCreateInfo& info);
            Shader* CreateShader(ShaderConfig& config);
            Sampler* CreateSampler(SamplerCreateInfo& info);

            void FreeGeometry(VulkanGeometry* geometry);
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

            VulkanDevice* device;
            VulkanSwapchain* swapchain;
            VulkanRenderpass* world_renderpass;
            VulkanRenderpass* ui_renderpass;
            // std::vector<std::string, VulkanRenderpass*> renderpasses;

            VkSurfaceKHR surface;
            VkInstance vulkan_instance;
            VkAllocationCallbacks* allocator;

            u32 obj_id;
    };

};
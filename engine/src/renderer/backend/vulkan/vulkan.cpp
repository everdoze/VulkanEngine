#include "vulkan.hpp"

#include "platform/platform.hpp"
#include "core/logger/logger.hpp"
#include "helpers.hpp"
#include "renderer/renderer_types.hpp"
#include "texture.hpp"
#include "material.hpp"

namespace Engine {

    FreelistNode* VulkanRendererBackend::UploadDataRange(VkCommandPool pool, VkFence fence, VkQueue queue, VulkanBuffer* buffer, u64 size, void* data) {
        // Create a host-visible staging buffer to upload to. Mark it as the source of the transfer.
        VkBufferUsageFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        VulkanBuffer staging = VulkanBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, flags, true, false);

        // Load the data into the staging buffer.
        staging.LoadData(0, size, 0, data);

        // Perform the copy from staging to the device local buffer.
        FreelistNode* allocation = buffer->Allocate(size);
        staging.CopyTo(pool, fence, queue, 0, buffer->handle, allocation->GetMemoryOffset(), allocation->GetSize());
        return allocation;
    }

    void VulkanRendererBackend::FreeDataRange(VulkanBuffer* buffer, u64 offset, u64 size) {
        return;
    };

    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback (
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
        void* user_data) {
        switch (message_severity) {
            default:
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                ERROR(callback_data->pMessage);
                break;

            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                INFO(callback_data->pMessage);
                break;

            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                WARN(callback_data->pMessage);
                break;

            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                TRACE(callback_data->pMessage);
                break;    
        }
        return VK_FALSE;
    };

    VulkanRendererBackend::VulkanRendererBackend(RendererSetup setup) : RendererBackend(setup) {
        object_vertex_buffer = nullptr;
        object_index_buffer = nullptr;
        allocator = nullptr;
        device = nullptr;
        swapchain = nullptr;
        current_frame = INVALID_ID;
    };

    b8 VulkanRendererBackend::Initialize() {
        recreating_swapchain = false;
        image_index = 0;
        framebuffer_generation = 0;
        framebuffer_last_generation = 0;

        VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
        app_info.apiVersion = VK_API_VERSION_1_3;
        app_info.pApplicationName = name.c_str();
        app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
        app_info.pEngineName = "Engine";
        app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);

        VkInstanceCreateInfo create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        create_info.pApplicationInfo = &app_info;

        std::vector<char*> extensions = Platform::GetRequiredExtensionsVK();
        extensions.push_back((char*)VK_KHR_SURFACE_EXTENSION_NAME);

        #if defined(_DEBUG)
            extensions.push_back((char*)VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // debug utilities

            DEBUG("Required extensions:");
            for (u32 i = 0; i < extensions.size(); ++i) {
                DEBUG(extensions[i]);
            }
        #endif

        create_info.enabledExtensionCount = extensions.size();
        create_info.ppEnabledExtensionNames = extensions.data();

        char** validation_layers = nullptr;
        u32 validation_layers_count = 0;
        #if defined(_DEBUG)
            DEBUG("Validation layers enabled due to debug mode. Enumerating...");

            // List of validation layers required VULKAN VALIDATION_LAYERS
            std::vector<char*> validation_layers_v;
            validation_layers_v.push_back((char*)"VK_LAYER_KHRONOS_validation");
            // validation_layers_v.push_back((char*)"VK_LAYER_LUNARG_api_dump");
            validation_layers_count = validation_layers_v.size();

            // Obtain a list of available validation layers
            u32 available_layer_count = 0;
            VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, 0));
            
            std::vector<VkLayerProperties> available_layers;
            available_layers.resize(available_layer_count);

            VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers.data()));

            // Verify that all required validation layers available.
            for (u32 i = 0; i < validation_layers_count; ++i) {
                DEBUG("Searching for layer: %s...", validation_layers_v[i]);
                b8 found = false;
                for (u32 j = 0; j < available_layer_count; ++j) {
                    std::string layer_name = validation_layers_v[i];
                    std::string available_layer = available_layers[j].layerName;
                    if (layer_name == available_layer) {
                        found = true;
                        DEBUG("Found!");
                        break;
                    }
                }

                if (!found) {
                    ERROR("Required validation layer is missing: %s", validation_layers_v[i]);
                    return false;
                }
            }
            validation_layers = validation_layers_v.data();
            DEBUG("All validation layers are present.");
        #endif

        create_info.enabledLayerCount = validation_layers_count;
        create_info.ppEnabledLayerNames = validation_layers;

        VK_CHECK(vkCreateInstance(&create_info, allocator, &vulkan_instance));
        DEBUG("Vulkan instance created successfully.");
        
        #if defined(_DEBUG)
            DEBUG("Creating Vulkan debugger...");

            u32 log_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;   

            VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
            debug_create_info.messageSeverity = log_severity;
            debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
                                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | 
                                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

            debug_create_info.pfnUserCallback = VulkanDebugCallback;                            

            PFN_vkCreateDebugUtilsMessengerEXT func = 
                (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkan_instance, "vkCreateDebugUtilsMessengerEXT");

            ASSERT_MSG(func, "Failed to create debug messenger!");
            VK_CHECK(func(vulkan_instance, &debug_create_info, allocator, &debug_messenger));
            DEBUG("Vulkan debugger created successfully.");
        #endif

        // Surface
        if (!Platform::CreateVulkanSurface(this)) {
            ERROR("Failed to create Vulkan surface!");
            return false;
        }
        DEBUG("Vulkan surface created successfully.");
        
        // Device
        device = VulkanDevice::CreateDevice(this);
        if (!device) {
            ERROR("Failed to create Vulkan device!")
            return false;
        }

        // Swapchain
        if (!SwapchainCreate(width, height)) {
            ERROR("Failed to create Vulkan swapchain!");
            return false;
        }

        // Renderpasses
        if (!RenderpassesCreate()) {
            ERROR("Failed to create Vulkan renderpasses!");
            return false;
        }

        // Swapchain framebuffers
        GenerateFramebuffers();

        // Command buffers
        DEBUG("Creating command buffers...");
        CreateCommandBuffers();

        // Sync objects (fences and semaphores)
        DEBUG("Creating sync objects...");
        CreateSyncObjects();

        // Buffers
        DEBUG("Creating buffers...");
        CreateBuffers();

        return true;
    };

    void VulkanRendererBackend::SetVulkanSurface(VkSurfaceKHR surface) {
        this->surface = surface;
    };

    void VulkanRendererBackend::Shutdown() {
        vkDeviceWaitIdle(device->logical_device);

        // Destroy buffers
        DEBUG("Destroying Vulkan buffers...");
        DestroyBuffers();
        
        // Destroy shader module
        DEBUG("Destroying Vulkan shader modules...");

        // Destroy sync objects
        DEBUG("Destroying Vulkan sync objects...");
        DestroySyncObjects();

        // Destroy command buffers
        DEBUG("Destroying Vulkan command buffers...");
        DestroyCommandBuffers();

        // Destroy renderpasses
        DEBUG("Destroying Vulkan renderpasses...");
        delete ui_renderpass;
        delete world_renderpass;

        // Destroy swapchain
        DEBUG("Destroying Vulkan swapchain...");
        delete swapchain;

        DEBUG("Destroying device...");
        delete device;

        // Destroy surface
        DEBUG("Destroying Vulkan surface...");
        Platform::DestroyVulkanSurface(this);
    };

    void VulkanRendererBackend::Resized(u16 width, u16 height) {
        cached_width = width;
        cached_height = height;
        framebuffer_generation++;

        DEBUG("Vulkan renderer backend->resized: w/h/gen: %i/%i/%llu", width, height, framebuffer_generation);
    };

    b8 VulkanRendererBackend::BeginFrame(f32 delta_time) {
        this->delta_time = delta_time;
        
        if (recreating_swapchain) {
            VkResult result = vkDeviceWaitIdle(device->logical_device);
            if (!IsVulkanResultSuccess(result)) {
                ERROR("VulkanRendererBackend::BeginFrame vkDeviceWaitIdle (1) failed: '%s'", VulkanResultString(result, true));
                return false;
            }
            INFO("Recreating swapchain, booting.");
            return false;
        }

        if (framebuffer_generation != framebuffer_last_generation) {
            VkResult result = vkDeviceWaitIdle(device->logical_device);
            if (!IsVulkanResultSuccess(result)) {
                ERROR("VulkanRendererBackend::BeginFrame vkDeviceWaitIdle (2) failed: '%s'", VulkanResultString(result, true));
                return false;
            }
            
            if (!SwapchainFullRecreate()) {
                return false;
            }

            // Update render area
            world_renderpass->OnResize(
                glm::vec4(0, 0, this->width, this->height)
            );
            ui_renderpass->OnResize(
                glm::vec4(0, 0, this->width, this->height)
            );

            DEBUG("Resized, booting.");
            return false;
        }

        if (!in_flight_fences[current_frame]->Wait(UINT64_MAX)) {
            WARN("In flight fence wait failure!");
            return false;
        }

        VkResult result = 
        swapchain->AcquireNextImageIndex(
            UINT64_MAX,
            image_available_semaphores[current_frame], 0, &image_index);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            SwapchainRecreate(width, height);
            return false;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            FATAL("Failed to acquire swapchain image.");
            return false;
        }

        VulkanCommandBuffer* command_buffer = graphics_command_buffers[image_index];
        command_buffer->Reset();
        command_buffer->Begin(false, false, false);

        // Dynamic state
        VkViewport viewport;
        viewport.x = 0.0f;
        viewport.y = (f32)height;
        viewport.width = (f32)width;
        viewport.height = -(f32)height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        // Scissor
        VkRect2D scissor;
        scissor.offset.x = scissor.offset.y = 0;
        scissor.extent.width = width;
        scissor.extent.height = height;

        vkCmdSetViewport(command_buffer->handle, 0, 1, &viewport);
        vkCmdSetScissor(command_buffer->handle, 0, 1, &scissor);

        world_renderpass->render_area.z = width;
        world_renderpass->render_area.w = height;

        return true;
    };

    b8 VulkanRendererBackend::BeginRenderpass(u8 renderpass_id) {
        VulkanRenderpass* renderpass = nullptr;
        VulkanFramebuffer* framebuffer = nullptr;
        VulkanCommandBuffer* command_buffer = graphics_command_buffers[image_index];
        
        switch (renderpass_id) {
            case (u8)BuiltinRenderpasses::WORLD: {
                renderpass = world_renderpass;
                framebuffer = swapchain->world_framebuffers[image_index];
            } break;

            case (u8)BuiltinRenderpasses::UI: {
                renderpass = ui_renderpass;
                framebuffer = swapchain->ui_framebuffers[image_index];
            } break;

            default: {
                ERROR("VulkanRendererBackend::BeginRenderpasses called with unknown renderpass id: %#02x", renderpass_id);
                return false;
            };
        }

        renderpass->Begin(command_buffer, framebuffer);

        return true;
    };

    b8 VulkanRendererBackend::EndRenderpass(u8 renderpass_id) {
        VulkanRenderpass* renderpass = nullptr;
        VulkanCommandBuffer* command_buffer = graphics_command_buffers[image_index];
        switch (renderpass_id) {
            case (u8)BuiltinRenderpasses::WORLD: {
                renderpass = world_renderpass;
            } break;

            case (u8)BuiltinRenderpasses::UI: {
                renderpass = ui_renderpass;
            } break;

            default: {
                ERROR("VulkanRendererBackend::EndRenderpass called with unknown renderpass id: %#02x", renderpass_id);
                return false;
            };
        }
        renderpass->End(command_buffer);
        return true;
    };

    b8 VulkanRendererBackend::EndFrame(f32 delta_time) {
        VulkanCommandBuffer* command_buffer = graphics_command_buffers[image_index];
        command_buffer->End();

        // Make sure the previous frame is not using this image (i.e. its fence is being waited on)
        if (images_in_flight[image_index] != VK_NULL_HANDLE) { 
            images_in_flight[image_index]->Wait(UINT64_MAX);
        }

        // Mark the image fence as in-use by this frame.
        images_in_flight[image_index] = in_flight_fences[current_frame];

        // Reset the fence for use on the next frame
        in_flight_fences[current_frame]->Reset();
        
        // Submit the queue and wait for the operation to complete.
        // Begin queue submission
        VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};

        // Command buffer(s) to be executed.
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer->handle;

        // The semaphore(s) to be signaled when the queue is complete.
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &queue_complete_semaphores[current_frame];

        // Wait semaphore ensures that the operation cannot begin until the image is available.
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &image_available_semaphores[current_frame];

        // Each semaphore waits on the corresponding pipeline stage to complete. 1:1 ratio.
        // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsequent colour attachment
        // writes from executing until the semaphore signals (i.e. one frame is presented at a time)
        VkPipelineStageFlags flags[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submit_info.pWaitDstStageMask = flags;

        VkResult result = vkQueueSubmit(
            device->graphics_queue,
            1,
            &submit_info,
            in_flight_fences[current_frame]->handle);
        if (result != VK_SUCCESS) {
            ERROR("vkQueueSubmit failed with result: %s", VulkanResultString(result, true));
            return false;
        }

        command_buffer->UpdateSubmitted();

        VkResult present_result = swapchain->Present(
            device->graphics_queue, 
            device->present_queue, 
            queue_complete_semaphores[current_frame], 
            image_index);

        if (present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR) {
            SwapchainRecreate(width, height);
            return false;
        } else if (present_result != VK_SUCCESS) {
            FATAL("Failed to acquire swapchain image!");
            return false;
        }

        return true;
    };

    i32 VulkanRendererBackend::FindMemoryIndex(u32 type_filter, u32 property_flags) {
        VkPhysicalDeviceMemoryProperties memory_properties;
        vkGetPhysicalDeviceMemoryProperties(this->device->physical_device, &memory_properties);

        for (u32 i = 0; i < memory_properties.memoryTypeCount; ++i) {
            // Check each memory type to see if its bit is set to 1.
            if (type_filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags) {
                return i;
            }
        }

        WARN("Unable to find suitable memory type!");
        return -1;
    };

    b8 VulkanRendererBackend::SwapchainCreate(u16 width, u16 height) {
        swapchain = new VulkanSwapchain(width, height);
        return swapchain->ready;
    };

    b8 VulkanRendererBackend::SwapchainRecreate(u16 width, u16 height) {
        if (swapchain) {
            delete swapchain;
        } else {
            ERROR("VulkanRendererBackend::SwapchainRecreate called when swapchain does not exists.");
        }
        return SwapchainCreate(width, height);
    };

    b8 VulkanRendererBackend::SwapchainFullRecreate() {
        // If already being recreated, do not try again.
        if (recreating_swapchain) {
            DEBUG("VulkanRendererBackend::SwapchainFullRecreate was called during recreaction...");
            return false;
        }

        // Detect if the window is too small to be drawn to
        if (width == 0 || height == 0) {
            DEBUG("VulkanRendererBackend::SwapchainFullRecreate called when window is < 1 in a dimension....");
            return false;
        }

        recreating_swapchain = true;

        // Wait for any operations to complete.
        vkDeviceWaitIdle(device->logical_device);

        for (u32 i = 0; i < swapchain->image_count; ++i) {
            images_in_flight[i] = nullptr;
        }

        SwapchainRecreate(cached_width, cached_height);

        width = cached_width;
        height = cached_height;

        world_renderpass->render_area.z = width;
        world_renderpass->render_area.w = height;

        cached_width = 0; 
        cached_height = 0;
        
        framebuffer_last_generation = framebuffer_generation;

        GenerateFramebuffers();

        CreateCommandBuffers();

        recreating_swapchain = false;

        return true;
    };

    void VulkanRendererBackend::RegenerateFramebuffers() {
        swapchain->RegenerateFramebuffers(width, height);
    };

    void VulkanRendererBackend::GenerateFramebuffers() {
        swapchain->GenerateUIFramebuffers(ui_renderpass);
        swapchain->GenerateWorldFramebuffers(world_renderpass);
    };

    b8 VulkanRendererBackend::RenderpassesCreate() {
        // World Renderpass
        world_renderpass = new VulkanRenderpass(
            "WorldRenderpass",
            glm::vec4(0, 0, this->width, this->height),
            glm::vec4(0.0f, 0.1f, 0.1f, 1.0f),
            1.0f, 0, // Depth, Stencil
            VulkanRenderPassClearFlag::CLEAR_COLOR_BUFFER | 
            VulkanRenderPassClearFlag::CLEAR_COLOR_DEPTH_BUFER | 
            VulkanRenderPassClearFlag::CLEAR_COLOR_STENCIL_BUFFER,
            false, true); // HasPrevPass, HasNextPass 
        // std:string buffer = WORLD_RENDERPASS_NAME;
        // renderpasses[buffer] = 
        // UI Renderpass
        ui_renderpass = new VulkanRenderpass(
            "UIRenderpass",
            glm::vec4(0, 0, this->width, this->height),
            glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
            1.0f, 0, // Depth, Stencil
            VulkanRenderPassClearFlag::CLEAR_NONE,
            true, false); // HasPrevPass, HasNextPass 
    
        return world_renderpass->ready && ui_renderpass->ready;
    };

    void VulkanRendererBackend::CreateSyncObjects() {
        image_available_semaphores.resize(swapchain->max_frames_in_flight);
        queue_complete_semaphores.resize(swapchain->max_frames_in_flight);
        in_flight_fences.reserve(swapchain->max_frames_in_flight);

        for (u8 i = 0; i < swapchain->max_frames_in_flight; ++i) {
            VkSemaphoreCreateInfo image_sp_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
            VkSemaphoreCreateInfo queue_sp_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

            vkCreateSemaphore(
                device->logical_device,
                &image_sp_create_info,
                allocator,
                &image_available_semaphores[i]);

            vkCreateSemaphore(
                device->logical_device,
                &image_sp_create_info,
                allocator,
                &queue_complete_semaphores[i]);

            in_flight_fences.push_back(new VulkanFence(true));
        }

        images_in_flight.resize(swapchain->image_count);
        for (u32 i = 0; i < swapchain->image_count; ++i) {
            images_in_flight[i] = nullptr;
        }

        DEBUG("Sync objects successfully created.");
    };

    void VulkanRendererBackend::DestroySyncObjects() {
        for (u32 i = 0; i < swapchain->max_frames_in_flight; ++i) {
            if (image_available_semaphores[i]) {
                vkDestroySemaphore(
                    device->logical_device,
                    image_available_semaphores[i],
                    allocator);
                image_available_semaphores[i] = nullptr;
            }

            if (queue_complete_semaphores[i]) {
                vkDestroySemaphore(
                    device->logical_device,
                    queue_complete_semaphores[i],
                    allocator);
                queue_complete_semaphores[i] = nullptr;
            }
        }

        image_available_semaphores.clear();
        queue_complete_semaphores.clear();

        for (u32 i = 0; i < in_flight_fences.size(); ++i) {
            delete in_flight_fences[i];
        }
 
        in_flight_fences.clear();
        images_in_flight.clear();
    };

    void VulkanRendererBackend::CreateCommandBuffers() {
        if (graphics_command_buffers.size()) {
            for (u32 i = 0; i < graphics_command_buffers.size(); ++i) {
                delete graphics_command_buffers[i];
            }
            graphics_command_buffers.clear();
        }

        if (!graphics_command_buffers.capacity()) {
            graphics_command_buffers.reserve(swapchain->image_count);
        }

        for (u32 i = 0; i < swapchain->image_count; ++i) {
            graphics_command_buffers.push_back(new VulkanCommandBuffer(device->graphics_command_pool, true));
        }

        DEBUG("Command buffers created successfully.");
    };

    void VulkanRendererBackend::DestroyCommandBuffers() {
        for (u32 i = 0; i < swapchain->image_count; ++i) {
            delete graphics_command_buffers[i];
        }
        graphics_command_buffers.clear();
    };


    b8 VulkanRendererBackend::CreateBuffers() {
        VkMemoryPropertyFlagBits memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        const u64 vertex_buffer_size = sizeof(Vertex3D) MB;

        // Geometry vertex buffer
        object_vertex_buffer = new VulkanBuffer(
            vertex_buffer_size,
            (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT),
            memory_property_flags, true, true);

        if (!object_vertex_buffer->ready) {
            ERROR("Failed to create object_vertex_buffer ... ");
            return false;
        }

        // Geometry index buffer
        const u64 index_buffer_size = sizeof(u32) MB;
        object_index_buffer = new VulkanBuffer(
            index_buffer_size,
            (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT),
            memory_property_flags, true, true);

        if (!object_index_buffer->ready) {
            ERROR("Failed to create object_index_buffer ... ");
            return false;
        }

        DEBUG("Vulkan buffers created successfully.");
        return true;
    };


    void VulkanRendererBackend::DestroyBuffers() {
        if (object_vertex_buffer) {
            delete object_vertex_buffer;
        }
        if (object_index_buffer) {
            delete object_index_buffer;
        }
    };
    
    void VulkanRendererBackend::DrawGeometry(GeometryRenderData data) {
        if (!data.geometry || data.geometry->GetInternalId() == INVALID_ID) {
            return;
        }
        VulkanGeometry* geometry = static_cast<VulkanGeometry*>(data.geometry);
        VulkanCommandBuffer* command_buffer = graphics_command_buffers[image_index];

        Material* material = data.geometry->GetMaterial();

        // switch (material->GetType())
        // {
        //     case MaterialType::WORLD: {
        //         material_shader->UseModel(data.model);
        //         material_shader->UseMaterial(material);
        //     } break;
        //     case MaterialType::UI: {
        //         ui_shader->UseModel(data.model);
        //         ui_shader->UseMaterial(material);
        //     } break;
        //     default:
        //         ERROR("VulkanRendererBackend::DrawGeometry - Unknown material type used...");
        //         return;
        // }

        VkDeviceSize offsets[1] = {geometry->GetVertexBufferOffset()};
        vkCmdBindVertexBuffers(command_buffer->handle, 0, 1, &object_vertex_buffer->handle, (VkDeviceSize*)offsets);

        if (geometry->GetIndexCount()) {
            // Bind index buffer at offset.
            vkCmdBindIndexBuffer(command_buffer->handle, object_index_buffer->handle, geometry->GetIndexBufferOffset(), VK_INDEX_TYPE_UINT32);

            // Issue the draw.
            vkCmdDrawIndexed(command_buffer->handle, geometry->GetIndexCount(), 1, 0, 0, 0);
        } else {
            vkCmdDraw(command_buffer->handle, geometry->GetVertexCount(), 1, 0, 0);
        }
    };

    Texture* VulkanRendererBackend::CreateTexture(TextureCreateInfo& info) {
        return CreateTextureInternal(info);
    };

    VulkanTexture* VulkanRendererBackend::CreateTextureInternal(TextureCreateInfo& info) {
        return new VulkanTexture(info);
    };

    Material* VulkanRendererBackend::CreateMaterial(MaterialCreateInfo& info) {
        VulkanMaterial* material = new VulkanMaterial(info);
        if (!material->AcquireInstanceResources()) {
            ERROR("Unable to acquire resources for world material: '%s'", material->GetName().c_str());
            delete material;
        };
        return material;
    };

    Geometry* VulkanRendererBackend::CreateGeometry(GeometryCreateInfo& info) {
        if (!info.vertex_count || !info.vertex_element_size || !info.vertices) {
            ERROR("No vertex data was supplied to VulkanRendererBackend::CreateGeometry.");
            return nullptr;
        } 
        
        VulkanGeometryCreateInfo create_info = {};
        
        create_info.vertex_count = info.vertex_count;
        create_info.vertex_size = info.vertex_element_size * info.vertex_count;

        create_info.index_count = info.index_count;
        create_info.index_size = info.index_element_size * info.index_count;
        


        create_info.vertex_memory = UploadDataRange(
            device->graphics_command_pool, 
            0, device->graphics_queue, 
            object_vertex_buffer, 
            create_info.vertex_size, info.vertices);  
        

        if (info.indices) {
            create_info.index_memory = UploadDataRange(
                device->graphics_command_pool, 
                0, device->graphics_queue, 
                object_index_buffer, 
                create_info.index_size, info.indices);
        }

        VulkanGeometry* g = new VulkanGeometry(info, create_info);

        g->UpdateGeneration();

        return g;
    };

    void VulkanRendererBackend::FreeGeometry(VulkanGeometry* geometry) {
        vkDeviceWaitIdle(device->logical_device);
        FreeDataRange(object_vertex_buffer, geometry->GetVertexBufferOffset(), geometry->GetVertexSize());
        if (geometry->GetIndexSize()) {
            FreeDataRange(object_index_buffer, geometry->GetIndexBufferOffset(), geometry->GetVertexSize());
        }
    };

    Shader* VulkanRendererBackend::CreateShader(ShaderConfig& config) {
        VulkanShaderConfig vk_config = {};
        
        vk_config.max_descriptor_set_count = 1024;
        if (config.renderpass_name == "Renderpass.Builtin.World") {
            vk_config.renderpass = world_renderpass;
        } else if (config.renderpass_name == "Renderpass.Builtin.UI") {
            vk_config.renderpass = ui_renderpass;
        }
        
        
        // HACK: не хорошо хардкоженые использовать
        vk_config.pool_sizes.push_back((VkDescriptorPoolSize){VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024});
        vk_config.pool_sizes.push_back((VkDescriptorPoolSize){VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4096});

        VulkanShaderDescrptorSetConfig global_descriptor = {};

        VkDescriptorSetLayoutBinding global_ubo_binding = {};
        global_ubo_binding.binding = (u32)VulkanShaderDescriptorBindingIndex::UBO;
        global_ubo_binding.descriptorCount = 1;
        global_ubo_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        global_ubo_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        global_descriptor.bindings.push_back(global_ubo_binding);
        global_descriptor.flags |= VulkanShaderDescriptorBindingFlags::UBO;
        vk_config.descriptor_sets[(u32)ShaderScope::GLOBAL] = global_descriptor;

        if (config.use_instances) {
            VulkanShaderDescrptorSetConfig instance_descriptor = {};

            VkDescriptorSetLayoutBinding instance_ubo_binding = {};
            instance_ubo_binding.binding = (u32)VulkanShaderDescriptorBindingIndex::UBO;
            instance_ubo_binding.descriptorCount = 1;
            instance_ubo_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            instance_ubo_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

            instance_descriptor.bindings.push_back(instance_ubo_binding);
            instance_descriptor.flags |= VulkanShaderDescriptorBindingFlags::UBO;
            vk_config.descriptor_sets[(u32)ShaderScope::INSTANCE] = instance_descriptor;
        }

        for (u32 i =  0; i < config.uniforms.size(); ++i) {
            if (config.uniforms[i].type == ShaderUniformType::SAMPLER) {
                VulkanShaderDescrptorSetConfig* descriptor_config = &vk_config.descriptor_sets[(u32)config.uniforms[i].scope];
                if (*descriptor_config != 0 && !(descriptor_config->flags & VulkanShaderDescriptorBindingFlags::SAMPLER)) {
                    VkDescriptorSetLayoutBinding sampler_binding = {};
                    sampler_binding.binding = (u32)VulkanShaderDescriptorBindingIndex::SAMPLER;
                    sampler_binding.descriptorCount = 1;
                    sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    sampler_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
                    descriptor_config->bindings.push_back(sampler_binding);
                    descriptor_config->flags |= VulkanShaderDescriptorBindingFlags::SAMPLER;
                } else {
                    descriptor_config->bindings[(u32)VulkanShaderDescriptorBindingIndex::SAMPLER].descriptorCount++;
                }
            }
        }

        return new VulkanShader(vk_config, config);
    };


};  
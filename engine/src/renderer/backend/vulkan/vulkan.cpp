#include "vulkan.hpp"

#include "platform/platform.hpp"
#include "core/logger/logger.hpp"
#include "vulkan_helpers.hpp"
#include "renderer/renderer_types.inl"
#include "vulkan_texture.hpp"

#define BUILTIN_SHADER_NAME_OBJECT "Builtin.MaterialShader"

namespace Engine {

    void VulkanRendererBackend::UploadDataRange(VkCommandPool pool, VkFence fence, VkQueue queue, VulkanBuffer* buffer, u64 offset, u64 size, void* data) {
        // Create a host-visible staging buffer to upload to. Mark it as the source of the transfer.
        VkBufferUsageFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        VulkanBuffer staging = VulkanBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, flags, true);

        // Load the data into the staging buffer.
        staging.LoadData(0, size, 0, data);

        // Perform the copy from staging to the device local buffer.
        staging.CopyTo(pool, fence, queue, 0, buffer->handle, offset, size);
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

            // List of validation layers required
            std::vector<char*> validation_layers_v;
            validation_layers_v.push_back((char*)"VK_LAYER_KHRONOS_validation");
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
                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT; //|
                            // VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                            // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;   

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
        device = Device::CreateDevice(this);
        if (!device) {
            ERROR("Failed to create Vulkan device!")
            return false;
        }

        // Swapchain
        if (!SwapchainCreate(width, height)) {
            ERROR("Failed to create Vulkan swapchain!");
            return false;
        }

        // Renderpass
        if (!RenderpassCreate()) {
            ERROR("Failed to create Vulkan renderpass!");
            return false;
        }

        // Swapchain framebuffers
        swapchain->GenerateFramebuffers(main_renderpass);

        // Command buffers
        DEBUG("Creating command buffers...");
        CreateCommandBuffers();

        // Sync objects (fences and semaphores)
        DEBUG("Creating sync objects...");
        CreateSyncObjects();

        // Create default shader
        DEBUG("Creating builtin object shader...");
        default_shader = new VulkanShader(BUILTIN_SHADER_NAME_OBJECT);
        if (!default_shader->ready) {
            ERROR("Error when creating default object shader...");
            return false;
        }

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

        // default_shader->ReleaseResources(obj_id);

        // Destroy buffers
        DEBUG("Destroying Vulkan buffers...");
        DestroyBuffers();
        
        // Destroy shader module
        DEBUG("Destroying Vulkan shader module...");
        delete default_shader;

        // Destroy sync objects
        DEBUG("Destroying Vulkan sync objects...");
        DestroySyncObjects();

        // Destroy command buffers
        DEBUG("Destroying Vulkan command buffers...");
        DestroyCommandBuffers();

        // Destroy framebuffers
        DEBUG("Destroying Vulkan framebuffers...");
        swapchain->DestroyFramebuffers();

        // Destroy renderpass
        DEBUG("Destroying Vulkan renderpass...");
        delete main_renderpass;

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

        main_renderpass->w = width;
        main_renderpass->h = height;

        main_renderpass->Begin(command_buffer);

        return true;
    };

    void VulkanRendererBackend::DrawGeometry() {

    };

    b8 VulkanRendererBackend::EndFrame(f32 delta_time) {
        VulkanCommandBuffer* command_buffer = graphics_command_buffers[image_index];

        main_renderpass->End(command_buffer);
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

        NextFrame();

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
        delete swapchain;
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
            delete images_in_flight[i];
        }

        swapchain->DestroyFramebuffers();

        SwapchainRecreate(cached_width, cached_height);

        width = cached_width;
        height = cached_height;

        main_renderpass->w = width;
        main_renderpass->h = height;

        cached_width = 0; 
        cached_height = 0;
        
        framebuffer_last_generation = framebuffer_generation;

        swapchain->GenerateFramebuffers(main_renderpass);

        CreateCommandBuffers();

        recreating_swapchain = false;

        return true;
    };

    b8 VulkanRendererBackend::RenderpassCreate() {
        main_renderpass = new VulkanRenderpass(
            0, 0, this->width, this->height,
            0.0f, 0.1f, 0.1f, 1.0f,
            1.0f, 0);
        return main_renderpass->ready;
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

        object_vertex_buffer = new VulkanBuffer(
            vertex_buffer_size,
            (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT),
            memory_property_flags, true);

        if (!object_vertex_buffer->ready) {
            ERROR("Failed to create object_vertex_buffer ... ");
            return false;
        }

        const u64 index_buffer_size = sizeof(u32) MB;
        object_index_buffer = new VulkanBuffer(
            index_buffer_size,
            (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT),
            memory_property_flags, true);

        if (!object_index_buffer->ready) {
            ERROR("Failed to create object_index_buffer ... ");
            return false;
        }

        DEBUG("Vulkan buffers created successfully.");
        return true;
    };


    void VulkanRendererBackend::DestroyBuffers() {
        delete object_vertex_buffer;
        delete object_index_buffer;
    };


    b8 VulkanRendererBackend::UpdateGlobalState(glm::mat4 projection, glm::mat4 view, glm::vec3 view_position, glm::vec4 ambient_colour, i32 mode) {
        default_shader->global_ubo.projection = projection;
        default_shader->global_ubo.view = view;

        default_shader->UpdateGlobalState();
        
        return true;
    };  

    void VulkanRendererBackend::DrawGeometry(GeometryRenderData data) {
        if (!data.geometry || data.geometry->GetInternalId() == INVALID_ID) {
            return;
        }
        VulkanGeometry* geometry = static_cast<VulkanGeometry*>(data.geometry);
        VulkanCommandBuffer* command_buffer = graphics_command_buffers[image_index];

        default_shader->Use();

        default_shader->UseModel(data.model);

        default_shader->UseMaterial(data.geometry->GetMaterial());

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
        Material* material = new Material(info);
        if (!default_shader->AcquireResources(material)) {
            ERROR("Unable to acquire resources for material: '%s'", material->GetName().c_str());
        };
        return material;
    };

    Geometry* VulkanRendererBackend::CreateGeometry(GeometryCreateInfo& info) {
        if (!info.vertices.size()) {
            ERROR("No vertex data was supplied to VulkanRendererBackend::CreateGeometry.");
            return nullptr;
        } 
        
        VulkanGeometryCreateInfo create_info = {};
        create_info.vertex_buffer_offset = vertex_buffer_offset;
        create_info.vertex_count = info.vertices.size();
        create_info.vertex_size = sizeof(Vertex3D) * info.vertices.size();

        create_info.index_buffer_offset = index_buffer_offset;
        create_info.index_count = info.indices.size();
        create_info.index_size = sizeof(u32) * info.indices.size();
        

        VulkanGeometry* g = new VulkanGeometry(info, create_info);

        UploadDataRange(
            device->graphics_command_pool, 
            0, device->graphics_queue, 
            object_vertex_buffer, g->GetVertexBufferOffset(), 
            g->GetVertexSize(), info.vertices.data());

        vertex_buffer_offset += g->GetVertexSize();

        if (info.indices.size()) {
            UploadDataRange(
                device->graphics_command_pool, 
                0, device->graphics_queue, 
                object_index_buffer, g->GetIndexBufferOffset(), 
                g->GetIndexSize(), info.indices.data());
        }

        g->UpdateGeneration();

        return g;
    };

    b8 VulkanRendererBackend::DestroyGeometry(VulkanGeometry* geometry) {
        
    };

    void VulkanRendererBackend::FreeGeometry(VulkanGeometry* geometry) {
        vkDeviceWaitIdle(device->logical_device);
        FreeDataRange(object_vertex_buffer, geometry->GetVertexBufferOffset(), geometry->GetVertexSize());
        if (geometry->GetIndexSize()) {
            FreeDataRange(object_index_buffer, geometry->GetIndexBufferOffset(), geometry->GetVertexSize());
        }
    };

};  
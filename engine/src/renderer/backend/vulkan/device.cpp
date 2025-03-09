#include "device.hpp"

#include "vulkan.hpp"
#include "core/logger/logger.hpp"
#include "helpers.hpp"
#include "platform/platform.hpp"

namespace Engine {

    VulkanDevice::VulkanDevice() {
        Platform::ZrMemory(&properties, sizeof(VkPhysicalDeviceProperties));
        Platform::ZrMemory(&features, sizeof(VkPhysicalDeviceFeatures));
        Platform::ZrMemory(&memory, sizeof(VkPhysicalDeviceMemoryProperties));
        Platform::ZrMemory(&logical_device, sizeof(VkDevice));
        Platform::ZrMemory(&physical_device, sizeof(VkPhysicalDevice));
    };

    VulkanDevice::~VulkanDevice() {
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();
        // Unset queues
        transfer_queue = nullptr;
        graphics_queue = nullptr;
        present_queue = nullptr;
        DEBUG("|_Queues unset.");

        // Destroying command pool
        DEBUG("|_Destroying command pools...");
        vkDestroyCommandPool(
            logical_device,
            graphics_command_pool,
            backend->GetVulkanAllocator());

        // Destroy logical device
        DEBUG("|_Destroying logical device...");
        if (logical_device) {
            vkDestroyDevice(logical_device, backend->GetVulkanAllocator());
            logical_device = nullptr;
        }

        DEBUG("|_Releasing physical device resources...");
        physical_device = nullptr;

        if (swapchain_support.formats) {
            Platform::FrMemory(swapchain_support.formats);
            swapchain_support.formats = nullptr;
            swapchain_support.format_count = 0;
        }

        if (swapchain_support.present_modes) {
            Platform::FrMemory(swapchain_support.present_modes);
            swapchain_support.present_modes = nullptr;
            swapchain_support.present_mode_count = 0;
        }

        Platform::ZrMemory(&swapchain_support.capabilities, sizeof(swapchain_support.capabilities));

        graphics_queue_index = -1;
        present_queue_index = -1;
        transfer_queue_index = -1;
    };

    VulkanDevice* VulkanDevice::CreateDevice(VulkanRendererBackend* backend) {
        VulkanDevice* new_device = new VulkanDevice();
        
        if (!new_device->SelectPhysicalDevice()) {
            return nullptr;
        }

        DEBUG("Creating logical device...");

        b8 present_shares_graphics_queue = new_device->graphics_queue_index == new_device->present_queue_index;
        b8 transfer_shares_graphics_queue = new_device->graphics_queue_index == new_device->transfer_queue_index;
        u32 index_count = 1;

        if (!present_shares_graphics_queue) {
            index_count++;
        }
        if (!transfer_shares_graphics_queue) {
            index_count++;
        }

        const u32 max_queue_count = 32;
        u32 indecies[max_queue_count];
        u8 index = 0;
        indecies[index++] = new_device->graphics_queue_index;
        if (!present_shares_graphics_queue) {
            indecies[index++] = new_device->present_queue_index;
        }
        if (!transfer_shares_graphics_queue) {
            indecies[index++] = new_device->transfer_queue_index;
        }

        VkDeviceQueueCreateInfo queues_create_infos[max_queue_count];
        Platform::ZrMemory(queues_create_infos, sizeof(VkDeviceQueueCreateInfo) * max_queue_count);
        for (u32 i = 0; i < index_count; ++i) {
            queues_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queues_create_infos[i].queueFamilyIndex = indecies[i];
            queues_create_infos[i].queueCount = 1;
            queues_create_infos[i].flags = 0;
            queues_create_infos[i].pNext = 0;
            f32 queue_priority = 1.0f;
            queues_create_infos[i].pQueuePriorities = &queue_priority;
        }

        // Request device features.
        // TODO: should be config driven
        VkPhysicalDeviceFeatures device_features = {};
        device_features.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo device_create_info = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        device_create_info.queueCreateInfoCount = index_count;
        device_create_info.pQueueCreateInfos = queues_create_infos;
        device_create_info.pEnabledFeatures = &device_features;
        device_create_info.enabledExtensionCount = 1;
        const char* extension_names = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        device_create_info.ppEnabledExtensionNames = &extension_names;

        VK_CHECK(vkCreateDevice(
            new_device->physical_device, 
            &device_create_info, 
            backend->GetVulkanAllocator(), 
            &new_device->logical_device));

        DEBUG("Logical device created successfully.");

        vkGetDeviceQueue(
            new_device->logical_device,
            new_device->graphics_queue_index,
            0,
            &new_device->graphics_queue);

        vkGetDeviceQueue(
            new_device->logical_device,
            new_device->present_queue_index,
            0,
            &new_device->present_queue);

        vkGetDeviceQueue(
            new_device->logical_device,
            new_device->transfer_queue_index,
            0,
            &new_device->transfer_queue); 

        // Create command pool
        VkCommandPoolCreateInfo cp_create_info = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        cp_create_info.queueFamilyIndex = new_device->graphics_queue_index;
        cp_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        VK_CHECK(vkCreateCommandPool(
            new_device->logical_device, 
            &cp_create_info, 
            backend->GetVulkanAllocator(), 
            &new_device->graphics_command_pool));

        return new_device;
    };

    b8 VulkanDevice::SelectPhysicalDevice() {
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();

        u32 physical_device_count = 0;
        VK_CHECK(vkEnumeratePhysicalDevices(backend->GetVulkanInstance(), &physical_device_count, 0));
        if (physical_device_count == 0) {
            FATAL("No devices supported by Vulkan found.");
            return false;
        }

        const u32 max_device_count = 32;
        VkPhysicalDevice physical_devices[max_device_count];
        VK_CHECK(vkEnumeratePhysicalDevices(backend->GetVulkanInstance(), &physical_device_count, physical_devices));

        for (u32 i = 0; i < physical_device_count; ++i) {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(physical_devices[i], &properties);

            VkPhysicalDeviceFeatures features;
            vkGetPhysicalDeviceFeatures(physical_devices[i], &features);

            VkPhysicalDeviceMemoryProperties memory;
            vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &memory);

            b8 supports_device_local_host_visible = false;
            for (u32 i = 0; i < memory.memoryTypeCount; ++i) {
                if ((memory.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0 &&
                    (memory.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0) {
                    supports_device_local_host_visible = true;
                    break;
                }
            }

            // TODO: These requirements should probably be driven by engine
            // configuration.
            VulkanPhysicalDeviceRequirements requirements;
            requirements.graphics = true;
            requirements.present = true;
            requirements.transfer = true;
            // NOTE: Enable this if compute will be required.
            // requirements.compute = true;
            requirements.sampler_anisotropy = true;
            requirements.discrete_gpu = true;
            requirements.device_extension_names.push_back((char*)VK_KHR_SWAPCHAIN_EXTENSION_NAME);

            VulkanPhysicalDeviceQueueFamilyInfo queue_info = {};
            b8 result = PhysicalDeviceMeetsRequirements(
                physical_devices[i],
                backend->GetVulkanSurface(),
                &properties,
                &features,
                &requirements,
                &queue_info,
                &this->swapchain_support
            );

            if (result) {
                DEBUG("Selected device: '%s'.", properties.deviceName);
                // GPU type, etc.
                switch (properties.deviceType) {
                    default:

                    case VK_PHYSICAL_DEVICE_TYPE_OTHER: {
                        DEBUG("GPU type is Unknown.");
                    } break;
                        
                    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: {
                        DEBUG("GPU type is Integrated.");
                    } break;

                    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: {
                        DEBUG("GPU type is Descrete.");
                    } break;

                    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: {
                        DEBUG("GPU type is Virtual.");
                    } break;
                        
                    case VK_PHYSICAL_DEVICE_TYPE_CPU: {
                        DEBUG("GPU type is CPU.");
                    } break;
                }

                DEBUG(
                    "GPU Driver version: %d.%d.%d",
                    VK_VERSION_MAJOR(properties.driverVersion),
                    VK_VERSION_MINOR(properties.driverVersion),
                    VK_VERSION_PATCH(properties.driverVersion));

                // Vulkan API version.
                DEBUG(
                    "Vulkan API version: %d.%d.%d",
                    VK_VERSION_MAJOR(properties.apiVersion),
                    VK_VERSION_MINOR(properties.apiVersion),
                    VK_VERSION_PATCH(properties.apiVersion));

                // Memory information
                for (u32 j = 0; j < memory.memoryHeapCount; ++j) {
                    f32 memory_size_gib = (((f32)memory.memoryHeaps[j].size) / 1024.0f / 1024.0f / 1024.0f);
                    if (memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                        DEBUG("Local GPU memory: %.2f GB", memory_size_gib);
                    } else {
                        DEBUG("Shared System memory: %.2f GB", memory_size_gib);
                    }
                }

                physical_device = physical_devices[i];
                graphics_queue_index = queue_info.graphics_family_index;
                present_queue_index = queue_info.present_family_index;
                transfer_queue_index = queue_info.transfer_family_index;
                // NOTE: set compute index here if needed.

                // Keep a copy of properties, features and memory info for later use.
                this->properties = properties;
                this->features = features;
                this->memory = memory;
                supports_device_local_host_visible = supports_device_local_host_visible;
                break;
            }
        }  

        if (!physical_device) {
            ERROR("No physical devices found which meet the requirements.");
            return false;
        }
        DEBUG("Physical device selected.");
        return true;
    };

    b8 VulkanDevice::PhysicalDeviceMeetsRequirements(
        VkPhysicalDevice physical_device, 
        VkSurfaceKHR surface, 
        VkPhysicalDeviceProperties* properties,
        VkPhysicalDeviceFeatures* features,
        VulkanPhysicalDeviceRequirements* requirements,
        VulkanPhysicalDeviceQueueFamilyInfo* out_queue_info,
        VulkanSwapchainSupportInfo* out_swapchain_support) {
        
        out_queue_info->compute_family_index = -1;
        out_queue_info->graphics_family_index = -1;
        out_queue_info->present_family_index = -1;
        out_queue_info->transfer_family_index = -1;

        // Discrete GPU check
        if (requirements->discrete_gpu) {
            if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                DEBUG("Device is not discrete GPU, and one required. Skipping");
                return false;
            }
        }

        u32 queue_family_count = 0;
        const u32 max_queue_family_count = 32;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, 0);
        VkQueueFamilyProperties queue_families[max_queue_family_count];
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);

        // Look at each queue and see what queues it supports
        DEBUG("Graphics | Present | Compute | Transfer | Name");
        u8 min_transfer_score = 255;
        for (u32 i = 0; i < queue_family_count; ++i) {
            u8 current_transfer_score = 0;

            // Graphics queue?
            if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                out_queue_info->graphics_family_index = i;
                ++current_transfer_score;
            }

            // Compute queue?
            if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
                out_queue_info->compute_family_index = i;
                ++current_transfer_score;
            }

            // Transfer queue?
            if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
                // Take the index if it is the current lowest. This increases the
                // liklihood that it is a dedicated transfer queue.
                if (current_transfer_score <= min_transfer_score) {
                    min_transfer_score = current_transfer_score;
                    out_queue_info->transfer_family_index = i;
                }
            }

            // Present queue?
            VkBool32 supports_present = VK_FALSE;
            VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &supports_present));
            if (supports_present) {
                out_queue_info->present_family_index = i;
            }

        }

        DEBUG("       %d |       %d |       %d |        %d | %s",
                out_queue_info->graphics_family_index != -1,
                out_queue_info->present_family_index != -1,
                out_queue_info->compute_family_index != -1,
                out_queue_info->transfer_family_index != -1,
                properties->deviceName);

        if ((!requirements->graphics || (requirements->graphics && out_queue_info->graphics_family_index != -1)) &&
            (!requirements->present || (requirements->present && out_queue_info->present_family_index != -1)) &&
            (!requirements->compute || (requirements->compute && out_queue_info->compute_family_index != -1)) &&
            (!requirements->transfer || (requirements->transfer && out_queue_info->transfer_family_index != -1))) {
            DEBUG("Device meets queue requirements.");
            TRACE("Graphics Family Index: %i", out_queue_info->graphics_family_index);
            TRACE("Present Family Index:  %i", out_queue_info->present_family_index);
            TRACE("Transfer Family Index: %i", out_queue_info->transfer_family_index);
            TRACE("Compute Family Index:  %i", out_queue_info->compute_family_index);

            // Query swapchain support.
            QuerySwapchainSupport(
                physical_device,
                surface,
                out_swapchain_support);

            if (out_swapchain_support->format_count < 1 || out_swapchain_support->present_mode_count < 1) {
                if (out_swapchain_support->formats) {
                    Platform::FrMemory(out_swapchain_support->formats);
                }
                if (out_swapchain_support->present_modes) {
                    Platform::FrMemory(out_swapchain_support->present_modes);
                }
                DEBUG("Required swapchain support not present, skipping device.");
                return false;
            }

            // Device extensions.
            if (requirements->device_extension_names.size()) {
                u32 available_extension_count = 0;
                VkExtensionProperties* available_extensions = 0;
                VK_CHECK(vkEnumerateDeviceExtensionProperties(
                    physical_device,
                    0,
                    &available_extension_count,
                    0));

                if (available_extension_count != 0) {
                    available_extensions = (VkExtensionProperties*) Platform::AllocMemory(sizeof(VkExtensionProperties) * available_extension_count);
                    VK_CHECK(vkEnumerateDeviceExtensionProperties(
                        physical_device,
                        0,
                        &available_extension_count,
                        available_extensions));

                    u32 required_extension_count = requirements->device_extension_names.size();
                    for (u32 i = 0; i < required_extension_count; ++i) {
                        b8 found = false;
                        for (u32 j = 0; j < available_extension_count; ++j) {
                            std::string extension_name = requirements->device_extension_names[i];
                            std::string available_extension_name = available_extensions[j].extensionName;

                            if (extension_name == available_extension_name) {
                                found = true;
                                break;
                            }
                        }

                        if (!found) {
                            DEBUG("Required extension not found: '%s', skipping device.", requirements->device_extension_names[i]);
                            Platform::FrMemory(available_extensions);
                            return false;
                        }
                    }
                }
                Platform::FrMemory(available_extensions);
            }

            // Sampler anisotropy
            if (requirements->sampler_anisotropy && !features->samplerAnisotropy) {
                DEBUG("Device does not support samplerAnisotropy, skipping.");
                return false;
            }

            // Device meets all requirements.
            return true;
        }

        return false;

    }


    void VulkanDevice::QuerySwapchainSupport(
        VkPhysicalDevice physical_device,
        VkSurfaceKHR surface,
        VulkanSwapchainSupportInfo* out_swapchain_support) {

        // Surface capabilities
        VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            physical_device,
            surface,
            &out_swapchain_support->capabilities));

        // Surface formats
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
            physical_device,
            surface,
            &out_swapchain_support->format_count,
            nullptr));

        if (out_swapchain_support->format_count != 0) {
            out_swapchain_support->formats = (VkSurfaceFormatKHR*)Platform::AllocMemory(sizeof(VkSurfaceFormatKHR) * out_swapchain_support->format_count);

            VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
                physical_device,
                surface,
                &out_swapchain_support->format_count,
                out_swapchain_support->formats
            ));
        }

        // Present modes
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
            physical_device,
            surface,
            &out_swapchain_support->present_mode_count,
            0
        ));

        if (out_swapchain_support->present_mode_count != 0) {
            out_swapchain_support->present_modes = (VkPresentModeKHR*)Platform::AllocMemory(sizeof(VkPresentModeKHR) * out_swapchain_support->present_mode_count);

            VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
                physical_device,
                surface,
                &out_swapchain_support->present_mode_count,
                out_swapchain_support->present_modes
            ));
        }
    };

    b8 VulkanDevice::DetectDepthFormat() {
        const u64 candidate_count = 3;
        VkFormat candidates[candidate_count] = {
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT};

        u8 channel_count[candidate_count] = {4, 4, 3};

        u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
        for (u32 i = 0; i < candidate_count; ++i) {
            VkFormatProperties properties;
            vkGetPhysicalDeviceFormatProperties(physical_device, candidates[i], &properties);

            if ((properties.linearTilingFeatures & flags) == flags) {
                depth_format = candidates[i];
                depth_channel_count = channel_count[i];
                return true;
            } else if ((properties.optimalTilingFeatures & flags) == flags) {
                depth_format = candidates[i];
                depth_channel_count = channel_count[i];
                return true;
            }
        }    
        
        depth_format = VK_FORMAT_UNDEFINED;
        return false;
    };
};
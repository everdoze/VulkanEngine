#pragma once

#include "defines.hpp"
#include <vulkan/vulkan.h>

namespace Engine {
    
    typedef struct VulkanPhysicalDeviceQueueFamilyInfo {
        u32 graphics_family_index;
        u32 present_family_index;
        u32 compute_family_index;
        u32 transfer_family_index;
    } VulkanPhysicalDeviceQueueFamilyInfo;

    typedef struct VulkanPhysicalDeviceRequirements {
        b8 graphics;
        b8 present;
        b8 compute;
        b8 transfer;
        std::vector<char*> device_extension_names;
        b8 sampler_anisotropy;
        b8 discrete_gpu;
    } VulkanPhysicalDeviceRequirements;

    typedef struct VulkanSwapchainSupportInfo {
        VkSurfaceCapabilitiesKHR capabilities;
        u32 format_count;
        VkSurfaceFormatKHR* formats;
        u32 present_mode_count;
        VkPresentModeKHR* present_modes;        
    } VulkanSwapchainSupportInfo;

    class Device {
        public:
            Device();
            ~Device();

            static Device* CreateDevice(class VulkanRendererBackend* backend);
            static void DestroyDevice();

            b8 SelectPhysicalDevice();
            b8 PhysicalDeviceMeetsRequirements(
                VkPhysicalDevice physical_device, 
                VkSurfaceKHR surface, 
                VkPhysicalDeviceProperties* properties,
                VkPhysicalDeviceFeatures* features,
                VulkanPhysicalDeviceRequirements* requirements,
                VulkanPhysicalDeviceQueueFamilyInfo* out_queue_info,
                VulkanSwapchainSupportInfo* out_swapchain_support);

            void QuerySwapchainSupport(
                VkPhysicalDevice physical_device,
                VkSurfaceKHR surface,
                VulkanSwapchainSupportInfo* out_swapchain_support);

            b8 DetectDepthFormat();

            VkPhysicalDevice physical_device;
            VkDevice logical_device;
            VulkanSwapchainSupportInfo swapchain_support;

            i32 graphics_queue_index;
            i32 present_queue_index;
            i32 transfer_queue_index;

            VkQueue graphics_queue;
            VkQueue present_queue;
            VkQueue transfer_queue;

            VkCommandPool graphics_command_pool;

            VkPhysicalDeviceProperties properties;
            VkPhysicalDeviceFeatures features;
            VkPhysicalDeviceMemoryProperties memory;

            VkFormat depth_format;

            b8 supports_device_local_host_visible;
    };

};
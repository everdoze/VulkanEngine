#include "swapchain.hpp"
#include "vulkan.hpp"
#include "vulkan_helpers.hpp"
#include "core/logger/logger.hpp"
#include "platform/platform.hpp"

namespace Engine {

    VulkanSwapchain::VulkanSwapchain(
        u32 width,
        u32 height) {
            
        VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());

        ready = false;
 
        VkExtent2D swapchain_extent = {width, height};

        // Choose a swap surface format
        DEBUG("Looking for preferred image format...");
        b8 found = false;
        for (u32 i = 0; i < backend->GetVulkanDevice()->swapchain_support.format_count; ++i) {
            VkSurfaceFormatKHR format = backend->GetVulkanDevice()->swapchain_support.formats[i];
            // Preferred formats
            if (format.format == VK_FORMAT_B8G8R8_UNORM && 
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                this->image_format = format;
                found = true;
                DEBUG("Preferred image format found.");
                break;
            }
        }

        if (!found) {
            this->image_format = backend->GetVulkanDevice()->swapchain_support.formats[0];
            DEBUG("Preferred image format not found. Using first available.");
        }

        // Present mode
        VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
        for (u32 i = 0; i < backend->GetVulkanDevice()->swapchain_support.present_mode_count; ++i) {
            VkPresentModeKHR mode = backend->GetVulkanDevice()->swapchain_support.present_modes[i];
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                present_mode = mode;
                break;
            }
        }

        auto device = backend->GetVulkanDevice();
        backend->GetVulkanDevice()->QuerySwapchainSupport(
            device->physical_device,
            backend->GetVulkanSurface(),
            &device->swapchain_support);

        // Swapchain extent
        if (device->swapchain_support.capabilities.currentExtent.width != UINT32_MAX) {
            swapchain_extent = device->swapchain_support.capabilities.currentExtent;
        }    

        VkExtent2D min = device->swapchain_support.capabilities.minImageExtent;
        VkExtent2D max = device->swapchain_support.capabilities.maxImageExtent;
        swapchain_extent.width = Clamp(swapchain_extent.width, min.width, max.width);
        swapchain_extent.height = Clamp(swapchain_extent.height, min.height, max.height);

        u32 image_count = device->swapchain_support.capabilities.minImageCount + 1;
        if (device->swapchain_support.capabilities.maxImageCount > 0 &&
            image_count > device->swapchain_support.capabilities.maxImageCount) {
            image_count = device->swapchain_support.capabilities.maxImageCount;
        }

        this->max_frames_in_flight = image_count - 1;

        // Swapchain create info
        VkSwapchainCreateInfoKHR swapchain_create_info = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
        swapchain_create_info.surface = backend->GetVulkanSurface();
        swapchain_create_info.minImageCount = image_count;
        swapchain_create_info.imageFormat = this->image_format.format;
        swapchain_create_info.imageColorSpace = this->image_format.colorSpace;
        swapchain_create_info.imageExtent = swapchain_extent;
        swapchain_create_info.imageArrayLayers = 1;
        swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        // Set queue family indecies
        if (device->graphics_queue_index == device->present_queue_index) {
            u32 queueFamilyIndecies[] = {
                (u32)device->graphics_queue_index,
                (u32)device->present_queue_index};

            swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchain_create_info.queueFamilyIndexCount = 2;
            swapchain_create_info.pQueueFamilyIndices = queueFamilyIndecies;    
        } else {
            swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapchain_create_info.queueFamilyIndexCount = 0;
            swapchain_create_info.pQueueFamilyIndices = 0;   
        }

        swapchain_create_info.preTransform = device->swapchain_support.capabilities.currentTransform;
        swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchain_create_info.presentMode = present_mode;
        swapchain_create_info.clipped = VK_TRUE;
        swapchain_create_info.oldSwapchain = 0;

        VK_CHECK(vkCreateSwapchainKHR(
            backend->GetVulkanDevice()->logical_device,
            &swapchain_create_info,
            backend->GetVulkanAllocator(),
            &this->handle));

        backend->SetCurrentFrame(0);
        this->image_count = 0;    

        VK_CHECK(vkGetSwapchainImagesKHR(
            backend->GetVulkanDevice()->logical_device, 
            this->handle, 
            &this->image_count, 
            0));

        this->images = (VkImage*)Platform::AMemory(sizeof(VkImage) * this->image_count);
        this->views = (VkImageView*)Platform::AMemory(sizeof(VkImageView) * this->image_count);

        VK_CHECK(vkGetSwapchainImagesKHR(
            backend->GetVulkanDevice()->logical_device, 
            this->handle, 
            &this->image_count, 
            this->images));

        for (u32 i = 0; i < this->image_count; ++i) {
            VkImageViewCreateInfo view_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            view_info.image = this->images[i];
            view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            view_info.format = this->image_format.format;
            view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            view_info.subresourceRange.baseMipLevel = 0;
            view_info.subresourceRange.levelCount = 1;
            view_info.subresourceRange.baseArrayLayer = 0;
            view_info.subresourceRange.layerCount = 1;

            VK_CHECK(vkCreateImageView(
                backend->GetVulkanDevice()->logical_device,
                &view_info,
                backend->GetVulkanAllocator(),
                &this->views[i]));
        }

        if (!backend->GetVulkanDevice()->DetectDepthFormat()) {
            FATAL("Failed to find supported depth format.");
        }

        // Create depth buffer
        this->depth_attachment = new VulkanImage(
            VK_IMAGE_TYPE_2D,
            swapchain_extent.width,
            swapchain_extent.height,
            backend->GetVulkanDevice()->depth_format,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            true,
            VK_IMAGE_ASPECT_DEPTH_BIT);

        DEBUG("Swapchain created successfully.");
        ready = true;
    }

    VulkanSwapchain::~VulkanSwapchain() {
        VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());

        vkDeviceWaitIdle(backend->GetVulkanDevice()->logical_device);

        delete depth_attachment;

        for (u32 i = 0; i < this->image_count; ++i) {
            vkDestroyImageView(
                backend->GetVulkanDevice()->logical_device,
                this->views[i],
                backend->GetVulkanAllocator());
        }

        vkDestroySwapchainKHR(
            backend->GetVulkanDevice()->logical_device,
            this->handle,
            backend->GetVulkanAllocator());
    };

    VkResult VulkanSwapchain::Present(
        VkQueue graphics_queue,
        VkQueue present_queue,
        VkSemaphore render_complete_semaphore,
        u32 present_image_index) {
        
        VkPresentInfoKHR present_info = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &render_complete_semaphore;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &this->handle;
        present_info.pImageIndices = &present_image_index;
        present_info.pResults = 0;

        return vkQueuePresentKHR(present_queue, &present_info);
    }

    VkResult VulkanSwapchain::AcquireNextImageIndex(u64 timeout_ns, VkSemaphore image_available_semaphore, VkFence fence, u32* out_index) {
        VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());
        return vkAcquireNextImageKHR(
            backend->GetVulkanDevice()->logical_device,
            this->handle,
            timeout_ns,
            image_available_semaphore,
            fence,
            out_index);
    };

    void VulkanSwapchain::GenerateFramebuffers(VulkanRenderpass* renderpass) {
        VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());
        framebuffers.reserve(image_count);
        for (u32 i = 0; i < this->image_count; ++i) {
            u32 attachment_count = 2;
            
            VkImageView attachments[] = {
                this->views[i],
                this->depth_attachment->view
            };

            framebuffers.push_back(
                new VulkanFramebuffer(
                    renderpass,
                    backend->GetFrameWidth(),
                    backend->GetFrameHeight(),
                    attachment_count,
                    attachments)
            );

        }
        
    };

    void VulkanSwapchain::DestroyFramebuffers() {
        for (u32 i = 0; i < this->image_count; ++i) {
            delete framebuffers[i];
        }
        framebuffers.clear();
    };

};
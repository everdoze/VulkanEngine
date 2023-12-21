#include "fence.hpp"

#include "vulkan.hpp"
#include "helpers.hpp"
#include "core/logger/logger.hpp"

namespace Engine {

    VulkanFence::VulkanFence(b8 create_signaled) {
        VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());

        this->is_signaled = create_signaled;

        VkFenceCreateInfo fence_create_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        if (create_signaled) {
            fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        }
        
        VK_CHECK(vkCreateFence(
            backend->GetVulkanDevice()->logical_device, 
            &fence_create_info, 
            backend->GetVulkanAllocator(), 
            &this->handle));

    };

    VulkanFence::~VulkanFence() {
        if (this->handle) {
            VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());
            vkDestroyFence(
                backend->GetVulkanDevice()->logical_device,
                this->handle,
                backend->GetVulkanAllocator());
            this->handle = nullptr;
        }
        this->is_signaled = false;
    };

    b8 VulkanFence::Wait(u64 timeout_ns) {
        if (!this->is_signaled) {
            VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());
            if (!this->handle) {
                u32 size = 10;
                size = 12;
            }
            VkResult result = vkWaitForFences(
                backend->GetVulkanDevice()->logical_device,
                1,
                &this->handle,
                true,
                timeout_ns);

            switch (result) {
                case VK_SUCCESS:
                    this->is_signaled = true;
                    return true;
                case VK_TIMEOUT:
                    WARN("vkWaitForFences: Timed out.");
                    break;
                case VK_ERROR_DEVICE_LOST:
                    ERROR("vkWaitForFences: VK_ERROR_DEVICE_LOST.");
                    break;
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                    ERROR("vkWaitForFences: VK_ERROR_OUT_OF_HOST_MEMORY.");
                    break;
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    ERROR("vkWaitForFences: VK_ERROR_OUT_OF_DEVICE_MEMORY.");
                    break;
                default:
                    ERROR("vkWaitForFences: An unknown error has occurred.");
                    break;
            }
        } else {
            return true;
        }

        return false;
    };

    void VulkanFence::Reset() {
        if (this->is_signaled) {
            VulkanRendererBackend* backend = static_cast<VulkanRendererBackend*>(RendererFrontend::GetBackend());
            VK_CHECK(vkResetFences(
                backend->GetVulkanDevice()->logical_device,
                1,
                &this->handle));
            this->is_signaled = false;    
        }
    };

};
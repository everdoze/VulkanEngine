#include "command_buffer.hpp"
#include "vulkan.hpp"

#include "vulkan_helpers.hpp"
#include "core/logger/logger.hpp"

#include "platform/platform.hpp"

namespace Engine {

    VulkanCommandBuffer::VulkanCommandBuffer(
        VkCommandPool pool,
        b8 is_primary) {
        Ref<VulkanRendererBackend> backend = Cast<VulkanRendererBackend>(RendererFrontend::GetBackend());
        
        this->pool = pool;

        VkCommandBufferAllocateInfo cb_allocate_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        cb_allocate_info.commandPool = pool;
        cb_allocate_info.level = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        cb_allocate_info.commandBufferCount = 1;

        this->state = VulkanCommandBufferState::NOT_ALLOCATED;
        VK_CHECK(vkAllocateCommandBuffers(
            backend->GetVulkanDevice()->logical_device,
            &cb_allocate_info,
            &this->handle));
        this->state = VulkanCommandBufferState::READY;
    };

    VulkanCommandBuffer::~VulkanCommandBuffer() {
        Ref<VulkanRendererBackend> backend = Cast<VulkanRendererBackend>(RendererFrontend::GetBackend());
        vkFreeCommandBuffers(
            backend->GetVulkanDevice()->logical_device,
            pool,
            1,
            &this->handle);

        this->pool = nullptr;
        this->handle = nullptr;
        this->state = VulkanCommandBufferState::NOT_ALLOCATED;
    };

    void VulkanCommandBuffer::Begin(
        b8 is_single_use,
        b8 is_renderpass_continue,
        b8 is_simultaneous_use) {
        
        VkCommandBufferBeginInfo cb_begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        cb_begin_info.flags = 0;

        if (is_single_use) {
            cb_begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        }

        if (is_renderpass_continue) {
            cb_begin_info.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        }

        if (is_simultaneous_use) {
            cb_begin_info.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        }

        VK_CHECK(vkBeginCommandBuffer(this->handle, &cb_begin_info));
        this->state = VulkanCommandBufferState::RECORDING;
    };

    void VulkanCommandBuffer::End() {
        VK_CHECK(vkEndCommandBuffer(this->handle));
        this->state = VulkanCommandBufferState::RECORDING_ENDED;
    };

    void VulkanCommandBuffer::UpdateSubmitted() {
        this->state = VulkanCommandBufferState::SUBMITTED;
    };

    void VulkanCommandBuffer::Reset() {
        this->state = VulkanCommandBufferState::READY;
    };

    void VulkanCommandBuffer::BeginSingleUse() {
        this->Begin(true, false, false);
    };

    void VulkanCommandBuffer::EndSingleUse(VkQueue queue) {
        this->End();

        VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &this->handle;

        VK_CHECK(vkQueueSubmit(queue, 1, &submit_info, 0));

        // Wait for finish
        VK_CHECK(vkQueueWaitIdle(queue));
    };

    void VulkanCommandBuffer::InRenderPass() {
        this->state = VulkanCommandBufferState::IN_RENDER_PASS;
    };

    void VulkanCommandBuffer::Recording() {
        this->state = VulkanCommandBufferState::RECORDING;
    };

};
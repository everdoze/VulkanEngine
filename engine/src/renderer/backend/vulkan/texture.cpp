#include "texture.hpp"
#include "buffer.hpp"
#include "renderer/renderer.hpp"
#include "renderer/backend/vulkan/vulkan.hpp"
#include "helpers.hpp"
#include "core/logger/logger.hpp"

namespace Engine {

    VulkanTexture::VulkanTexture(TextureCreateInfo& info) : Texture(info) {
        
        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();

        VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;

        VkDeviceSize image_size = info.width * info.height * info.channel_count;

        VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VkMemoryPropertyFlags memory_prop_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        VulkanBuffer buffer = VulkanBuffer(
            image_size, usage,
            memory_prop_flags, true, false);

        buffer.LoadData(0, image_size, 0, info.pixels);

        this->image = new VulkanImage(
            VK_IMAGE_TYPE_2D,
            width,
            height,
            image_format,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
            VK_IMAGE_USAGE_SAMPLED_BIT | 
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            true, VK_IMAGE_ASPECT_COLOR_BIT
        );

        VkCommandPool pool = backend->GetVulkanDevice()->graphics_command_pool;
        VkQueue queue = backend->GetVulkanDevice()->graphics_queue;
        VulkanCommandBuffer command_buffer = VulkanCommandBuffer(pool, true);
        command_buffer.BeginSingleUse();

        this->image->TransitionLayout(
            &command_buffer,
            image_format,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        );
        
        this->image->CopyFromBuffer(
            buffer.handle,
            &command_buffer
        );

        this->image->TransitionLayout(
            &command_buffer,
            image_format,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        command_buffer.EndSingleUse(queue);

        this->has_transparency = has_transparency;
        this->generation++;
    };

    VulkanTexture::~VulkanTexture() {
        delete this->image;
    };

};

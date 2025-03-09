#include "texture.hpp"
#include "buffer.hpp"
#include "renderer/renderer.hpp"
#include "renderer/backend/vulkan/vulkan.hpp"
#include "helpers.hpp"
#include "core/logger/logger.hpp"

namespace Engine {
    VulkanTexture::VulkanTexture(TextureCreateInfo& info) : Texture(info) {
        image_format = ChannelCountToFormat(info.channel_count);

        if (flags & TextureFlag::IS_WRITEABLE) {
            CreateWriteableTexture(info);
        }

        CreateReadonlyTexture(info);  
    };

    void VulkanTexture::CreateReadonlyTexture(TextureCreateInfo& info) {
        this->image = new VulkanImage(
            VK_IMAGE_TYPE_2D,
            width,
            height,
            image_format,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | 
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
            VK_IMAGE_USAGE_SAMPLED_BIT | 
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            true, VK_IMAGE_ASPECT_COLOR_BIT
        );

        WriteData(info.pixels);
    };
    
    void VulkanTexture::CreateWriteableTexture(TextureCreateInfo& info) {
        this->image = new VulkanImage(
            VK_IMAGE_TYPE_2D,
            width,
            height,
            image_format,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | 
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
            VK_IMAGE_USAGE_SAMPLED_BIT | 
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            true, VK_IMAGE_ASPECT_COLOR_BIT
        );

        WriteData(info.pixels);
    };

    VulkanTexture::VulkanTexture(TextureCreateInfo& info, VulkanTextureImageInfo& image_info) : Texture(info) {
        this->image_format = image_info.format;

        this->image = new VulkanImage(
            width,
            height,
            image_info.image,
            image_info.format,
            true, VK_IMAGE_ASPECT_COLOR_BIT
        );

        this->generation++;
    };

    VulkanTexture::VulkanTexture(TextureCreateInfo& info, VulkanImage* image) : Texture(info) {
        this->image_format = image->format;
        this->image = image;
        this->generation++;
    };

    VulkanTexture::~VulkanTexture() {
        if (this->image) {
            delete this->image;
        }
    }

    void VulkanTexture::WriteData(const u8* pixels, u32 offset, u32 size) {
        // if (!(this->flags & TextureFlag::IS_WRITEABLE)) {
        //     return ERROR("VulkanTexture::WriteData called for non writable texture.");
        // }

        VulkanRendererBackend* backend = VulkanRendererBackend::GetInstance();

        VkDeviceSize image_size = width * height * channel_count;
        VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VkMemoryPropertyFlags memory_prop_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        VulkanBuffer buffer = VulkanBuffer(
            image_size, usage,
            memory_prop_flags, true, false);

        buffer.LoadData(0, image_size, 0, pixels);

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

        this->generation++;
    };

    void VulkanTexture::Resize(u32 width, u32 height) {
        if (!(this->flags & TextureFlag::IS_WRITEABLE)) {
            return ERROR("VulkanTexture::Resize called for non writable texture.");
        }

        this->image = new VulkanImage(
            VK_IMAGE_TYPE_2D,
            width,
            height,
            image_format,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | 
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
            VK_IMAGE_USAGE_SAMPLED_BIT | 
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            true, VK_IMAGE_ASPECT_COLOR_BIT
        );

        this->generation++;
    };

    VkFormat VulkanTexture::ChannelCountToFormat(u8 channel_count, VkFormat default_format) {
        switch (channel_count) {
            case 1:
                return VK_FORMAT_R8_UNORM;
            case 2:
                return VK_FORMAT_R8G8_UNORM;
            case 3:
                return VK_FORMAT_R8G8B8_UNORM;
            case 4:
                return VK_FORMAT_R8G8B8A8_UNORM;
            default:
                return default_format;
        }
    };
};

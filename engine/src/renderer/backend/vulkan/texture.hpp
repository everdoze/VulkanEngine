#pragma once

#include "defines.hpp"
#include "resources/texture/texture.hpp"
#include "image.hpp"

namespace Engine {

    struct VulkanTextureImageInfo {
        VkImage image;
        VkFormat format;
    };

    class VulkanTexture : public Texture {
        public:
            VulkanTexture(TextureCreateInfo& info);
            VulkanTexture(TextureCreateInfo& info, VulkanTextureImageInfo& image_info);
            VulkanTexture(TextureCreateInfo& info, VulkanImage* image);
            
            ~VulkanTexture();

            VulkanImage* GetImage() { return image; };

            void WriteData(const u8* pixels, u32 offset = 0, u32 size = 0) override;
            void Resize(u32 width, u32 height) override;

        protected:
            VulkanImage* image;
            VkFormat image_format;

            VkFormat ChannelCountToFormat(u8 channel_count, VkFormat default_format = VK_FORMAT_R8G8B8A8_UNORM);

            void CreateReadonlyTexture(TextureCreateInfo& info);
            void CreateWriteableTexture(TextureCreateInfo& info);
    };

};
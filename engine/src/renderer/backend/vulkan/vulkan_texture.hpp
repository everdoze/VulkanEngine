#pragma once

#include "defines.hpp"
#include "resources/texture/texture.hpp"
#include "image.hpp"

namespace Engine {

    class VulkanTexture : public Texture {
        public:
            VulkanTexture(
                std::string name,
                u32 width,
                u32 height,
                u8 channel_count,
                u8 has_transparency,
                u8* pixels
            );

            ~VulkanTexture();

            Ref<VulkanImage> GetImage() { return image; };
            VkSampler& GetSampler() { return sampler; };
        
        protected:
            Ref<VulkanImage> image;
            VkSampler sampler;
    };

};
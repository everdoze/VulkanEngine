#pragma once

#include "defines.hpp"
#include "resources/texture/texture.hpp"
#include "image.hpp"

namespace Engine {

    class VulkanTexture : public Texture {
        public:
            VulkanTexture(TextureCreateInfo& info);
            ~VulkanTexture();

            VulkanImage* GetImage() { return image; };
            VkSampler& GetSampler() { return sampler; };
        
        protected:
            VulkanImage* image;
            VkSampler sampler;
    };

};
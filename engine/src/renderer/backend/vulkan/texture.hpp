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
        
        protected:
            VulkanImage* image;
    };

};
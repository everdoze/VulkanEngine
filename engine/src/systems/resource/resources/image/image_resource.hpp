#pragma once

#include "defines.hpp"
#include "systems/resource/resources/base/resource.hpp"

namespace Engine {

    class ImageResource : public Resource {
        public:
            ImageResource(
                u32 loader_id, std::string name, 
                std::string full_path, u8* pixels,
                u8 channel_count, u32 width, u32 height
            );
            ~ImageResource();

            u8* GetPixels() { return pixels; };
            u8 GetChannelCount() { return channel_count; };
            u32 GetWidth() { return width; };
            u32 GetHeight() { return height; };
            std::pair<u32, u32> GetSize() { return std::pair<u32, u32>(width, height); };
            b8 HasTransparency() { return has_transparency; };

        protected:
            u8* pixels;
            u8 channel_count;
            u32 width;
            u32 height;
            b8 has_transparency;
    };

}
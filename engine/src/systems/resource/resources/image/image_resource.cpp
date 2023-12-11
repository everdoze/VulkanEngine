#include "image_resource.hpp"

#include "vendor/stb_image/stb_image.h"

namespace Engine {

    ImageResource::ImageResource(
        u32 loader_id, std::string name, 
        std::string full_path, u8* pixels,
        u8 channel_count, u32 width, u32 height) : Resource(loader_id, name, full_path) {
        this->pixels = pixels;
        this->channel_count = channel_count;
        this->width = width;
        this->height = height;
        if (pixels) {
            u64 total_size = width * height * channel_count;
            for (u64 i = 0; i < total_size; i += channel_count) {
                u8 alpha = this->pixels[i + 3];
                if (alpha < 255) {
                    has_transparency = true;
                    break;
                }
            }
        }
    };

    ImageResource::~ImageResource() {
        if (this->pixels) {
            stbi_image_free(this->pixels);
        }
    };

}
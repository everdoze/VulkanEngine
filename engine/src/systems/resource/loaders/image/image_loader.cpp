#include "image_loader.hpp"

#include "core/logger/logger.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image/stb_image.h"

#include "systems/resource/resources/image/image_resource.hpp"

namespace Engine {
    ImageLoader::ImageLoader(u32 id, std::string type_path, std::string custom_type) : ResourceLoader(id, ResourceType::IMAGE, type_path, custom_type) {};
    ImageLoader::ImageLoader(u32 id, std::string type_path) : ResourceLoader(id, ResourceType::IMAGE, type_path) {};

    Resource* ImageLoader::Load(std::string name) {
        std::string file_path = StringFormat("%s/%s.%s", type_path.c_str(), name.c_str(), "png");
        stbi_set_flip_vertically_on_load(true);

        const u8 required_channel_count = 4;

        i32 width;
        i32 height;
        i32 channel_count;
        b8 has_transparency = false;
        u8* data = stbi_load(
            file_path.c_str(),
            &width, &height,
            &channel_count,
            required_channel_count);
        
        ImageResource* image = nullptr;

        image = new ImageResource(id, name, file_path, data, required_channel_count, width, height);

        if (stbi_failure_reason()) {
            WARN("ImageLoader::Load failed to load file '%s' : %s", file_path.c_str(), stbi_failure_reason());
            stbi__err(0, 0);
        }

        return image;
    };

}

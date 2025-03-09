#include "image_loader.hpp"

#include "core/logger/logger.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image/stb_image.h"

#include "systems/resource/resources/image/image_resource.hpp"

namespace Engine {
    ImageLoader::ImageLoader(u32 id, std::string type_path, std::string custom_type) : ResourceLoader(id, ResourceType::IMAGE, type_path, custom_type) {};
    ImageLoader::ImageLoader(u32 id, std::string type_path) : ResourceLoader(id, ResourceType::IMAGE, type_path) {};

    Resource* ImageLoader::Load(std::string name) {
        stbi_set_flip_vertically_on_load(true);

        #define IMAGE_EXTENSION_COUNT 4
        b8 found = false;
        std::string file_path;
        std::string extensions[IMAGE_EXTENSION_COUNT] = {".tga", ".png", ".jpg", ".bmp"};
        for (u32 i = 0; i < IMAGE_EXTENSION_COUNT; ++i) {
            file_path = StringFormat("%s/%s%s", type_path.c_str(), name.c_str(), extensions[i].c_str());
            if (FileSystem::FileExists(file_path)) {
                found = true;
                break;
            }
        }

        if (!found) {
            ERROR("ImageLoader::Load - file not found for '%s'", name.c_str());
            return nullptr;
        }

        const u8 required_channel_count = 4;

        i32 width;
        i32 height;
        i32 channel_count;

        u8* data = stbi_load(
            file_path.c_str(),
            &width, &height,
            &channel_count,
            required_channel_count);
        
        ImageResource* image = nullptr;

        image = new ImageResource(id, name, file_path, data, required_channel_count, width, height);

        if (stbi_failure_reason() && !data) {
            ERROR("ImageLoader::Load failed to load file '%s' : %s", file_path.c_str(), stbi_failure_reason());
            stbi__err(0, 0);
        }

        return image;
    };

}

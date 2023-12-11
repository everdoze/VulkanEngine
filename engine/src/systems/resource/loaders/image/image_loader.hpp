#pragma once

#include "systems/resource/loaders/base/resource_loader.hpp"

namespace Engine {

    class ImageLoader : public ResourceLoader {
        public:
            ImageLoader(u32 id, std::string type_path, std::string custom_type);
            ImageLoader(u32 id, std::string type_path);

            Resource* Load(std::string name);
    };

} 

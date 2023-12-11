#pragma once

#include "systems/resource/loaders/base/resource_loader.hpp"

namespace Engine {

    class BinaryLoader : public ResourceLoader {
        public:
            BinaryLoader(u32 id, std::string type_path, std::string custom_type);
            BinaryLoader(u32 id, std::string type_path);

            Resource* Load(std::string name);
    };

} 

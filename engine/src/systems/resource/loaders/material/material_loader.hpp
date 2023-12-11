#pragma once

#include "systems/resource/loaders/base/resource_loader.hpp"

namespace Engine {

    class MaterialLoader : public ResourceLoader {
        public:
            MaterialLoader(u32 id, std::string type_path, std::string custom_type);
            MaterialLoader(u32 id, std::string type_path);

            Resource* Load(std::string name);
    };

} 

#pragma once

#include "systems/resource/loaders/base/resource_loader.hpp"

namespace Engine {

    class ShaderLoader : public ResourceLoader {
        public:
            ShaderLoader(u32 id, std::string type_path, std::string custom_type);
            ShaderLoader(u32 id, std::string type_path);

            Resource* Load(std::string name);
    };

} 

#pragma once

#include "defines.hpp"

#include "systems/resource/resources/base/resource.hpp"

namespace Engine {

    enum class ResourceType {
        UNKNOWN = 0,
        TEXT = 1,
        BINARY = 2,
        IMAGE = 3,
        MATERIAL = 4,
        STATIC_MESH = 5,
        CUSTOM = 6
    };

    class ResourceLoader {
        public:
            ResourceLoader(u32 id, ResourceType type, std::string type_path, std::string custom_type);
            ResourceLoader(u32 id, ResourceType type, std::string type_path);
            ~ResourceLoader();

            virtual Resource* Load(std::string name) = 0;

        protected:
            u32 id;
            ResourceType type;
            std::string custom_type;
            std::string type_path;

    };

}
#pragma once

#include "defines.hpp"

#include "systems/resource/resources/base/resource.hpp"

namespace Engine {

    enum class ResourceType {
        UNKNOWN,
        TEXT,
        BINARY,
        IMAGE,
        MATERIAL,
        MESH,
        SHADER,
        CUSTOM
    };

    class ResourceLoader {
        public:
            ResourceLoader(u32 id, ResourceType type, std::string type_path, std::string custom_type);
            ResourceLoader(u32 id, ResourceType type, std::string type_path);
            virtual ~ResourceLoader() = default;

            virtual Resource* Load(std::string name) = 0;

        protected:
            u32 id;
            ResourceType type;
            std::string custom_type;
            std::string type_path;

    };

}
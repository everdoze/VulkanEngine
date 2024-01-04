#pragma once

#include "defines.hpp"

#include "loaders/base/resource_loader.hpp"

#include "systems/resource/resources/base/resource.hpp"
#include "systems/resource/resources/image/image_resource.hpp"
#include "systems/resource/resources/material/material_resource.hpp"
#include "systems/resource/resources/binary/binary_resource.hpp"
#include "systems/resource/resources/shader/shader_resource.hpp"
#include "systems/resource/resources/mesh/mesh_resource.hpp"

#include "core/logger/logger.hpp"
#include "loaders/image/image_loader.hpp"
#include "loaders/material/material_loader.hpp"
#include "loaders/binary/binary_loader.hpp"
#include "loaders/shader/shader_loader.hpp"
#include "loaders/mesh/mesh_loader.hpp"

namespace Engine {

    class ResourceSystem {
        public:
            ResourceSystem(std::string base_path);
            ~ResourceSystem();


            static b8 Initialize(std::string base_path);
            static void Shutdown();
            static ResourceSystem* GetInstance() { return instance; };
            
            std::string CreateLoaderPath(std::string path);
            std::string CreateLoaderPath();

            void RegisterLoader(ResourceType type, ResourceLoader* loader);
            void RegisterLoader(std::string type, ResourceLoader* loader);

            Resource* LoadResource(ResourceType type, std::string name);
            Resource* LoadResource(std::string type, std::string name);

            std::string GetLoaderTypeName(ResourceType type);

        protected:
            static ResourceSystem* instance;
            std::unordered_map<ResourceType, ResourceLoader*> registered_loaders; 
            std::unordered_map<std::string, ResourceLoader*> registered_custom_loaders;
            std::string base_path;

    };

}
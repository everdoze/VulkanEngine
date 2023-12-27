#pragma once

#include "defines.hpp"

#include "loaders/base/resource_loader.hpp"

#include "systems/resource/resources/base/resource.hpp"
#include "systems/resource/resources/image/image_resource.hpp"
#include "systems/resource/resources/material/material_resource.hpp"
#include "systems/resource/resources/binary/binary_resource.hpp"
#include "systems/resource/resources/shader/shader_resource.hpp"

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
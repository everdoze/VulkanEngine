#include "resource_system.hpp"

#include "core/logger/logger.hpp"
#include "loaders/image/image_loader.hpp"
#include "loaders/material/material_loader.hpp"
#include "loaders/binary/binary_loader.hpp"
#include "loaders/shader/shader_loader.hpp"

namespace Engine {
    
    ResourceSystem* ResourceSystem::instance = nullptr;

    b8 ResourceSystem::Initialize(std::string base_path) {
        if (!instance) {
            instance = new ResourceSystem(base_path);
        }
        return true;
    };

    void ResourceSystem::Shutdown() {
        if (instance) {
            delete instance;
        }
    };

    std::string ResourceSystem::CreateLoaderPath(std::string path) {
        return StringFormat("%s%s", base_path.c_str(), path.c_str());
    };

    std::string ResourceSystem::CreateLoaderPath() {
        return base_path;
    };

    ResourceSystem::ResourceSystem(std::string base_path) {
        this->base_path = base_path;

        // Image loader
        RegisterLoader(ResourceType::IMAGE, new ImageLoader(registered_loaders.size(), CreateLoaderPath("/textures")));

        // Material loader
        RegisterLoader(ResourceType::MATERIAL, new MaterialLoader(registered_loaders.size(), CreateLoaderPath("/materials")));

        // Binary loader
        RegisterLoader(ResourceType::BINARY, new BinaryLoader(registered_loaders.size(), CreateLoaderPath()));

        // Shader loader
        RegisterLoader(ResourceType::SHADER, new ShaderLoader(registered_loaders.size(), CreateLoaderPath("/shaders")));
    };  

    ResourceSystem::~ResourceSystem() {
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wdelete-abstract-non-virtual-dtor"
        for (auto [key, loader] : registered_loaders) {
            delete loader;
        }
        for (auto [key, loader] : registered_custom_loaders) {
            delete loader;
        }
        #pragma clang diagnostic pop
    };

    void ResourceSystem::RegisterLoader(ResourceType type, ResourceLoader* loader) {
        if (loader) {
            registered_loaders[type] = loader;
        }
    };

    void ResourceSystem::RegisterLoader(std::string type, ResourceLoader* loader) {
        if (loader) {
            registered_custom_loaders[type] = loader;
        }
    };

    std::string ResourceSystem::GetLoaderTypeName(ResourceType type) {
        switch (type) {
            case ResourceType::TEXT: {
                return "TEXT";
            }
            case ResourceType::STATIC_MESH: {
                return "STATIC_MESH";
            }
            case ResourceType::IMAGE: {
                return "IMAGE";
            }
            case ResourceType::MATERIAL: {
                return "MATERIAL";
            }
            case ResourceType::BINARY: {
                return "BINARY";
            }
            default: {
                return "UNKNOWN";
            }
                
        }
    };

    Resource* ResourceSystem::LoadResource(ResourceType type, std::string name) {
        if (registered_loaders[type]) {
            return registered_loaders[type]->Load(name);
        }
        ERROR("No loader was found for type '%s'.", GetLoaderTypeName(type).c_str());
        return nullptr;
    };

    Resource* ResourceSystem::LoadResource(std::string type, std::string name) {
        if (registered_custom_loaders[type]) {
            return registered_custom_loaders[type]->Load(name);
        }
        ERROR("No loader was found for custom type '%s'.", type.c_str());
        return nullptr;
    };

} 

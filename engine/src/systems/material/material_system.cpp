#include "material_system.hpp"
#include "systems/texture/texture_system.hpp"
#include "renderer/renderer.hpp"

#include "core/logger/logger.hpp"
#include "core/utils/string.hpp"

#include "systems/resource/resource_system.hpp"

namespace Engine {

    MaterialSystem* MaterialSystem::instance = nullptr;

    MaterialSystem::MaterialSystem() {
       CreateDefaultMaterials();
    };

    MaterialSystem::~MaterialSystem() {
        DestroyDefaultMaterials();

        for (auto& [key, material] : registered_materials) { 
            delete material;
        } 

        registered_materials.clear();
    };

    void MaterialSystem::CreateDefaultMaterials() {
        MaterialCreateInfo mat_create_info = {};
        mat_create_info.name = DEFAULT_MATERIAL_NAME;
        mat_create_info.diffuse_color = glm::vec4(1, 1, 1, 1);
        mat_create_info.use = TextureUse::MAP_DIFFUSE;
        mat_create_info.texture = TextureSystem::GetInstance()->GetDefaultTexture();
        mat_create_info.type = MaterialType::WORLD;

        default_material = RendererFrontend::GetInstance()->CreateMaterial(mat_create_info);

        mat_create_info.name = DEFAULT_MATERIAL_NAME;
        mat_create_info.diffuse_color = glm::vec4(1, 1, 1, 1);
        mat_create_info.use = TextureUse::MAP_DIFFUSE;
        mat_create_info.texture = TextureSystem::GetInstance()->GetDefaultTexture();
        mat_create_info.type = MaterialType::UI;

        default_ui_material = RendererFrontend::GetInstance()->CreateMaterial(mat_create_info);
    };

    void MaterialSystem::DestroyDefaultMaterials() {
        delete default_material;
        delete default_ui_material;
    };

    b8 MaterialSystem::Initialize() {
        if (!instance) {
            instance = new MaterialSystem();
            return true;
        }
        WARN("Material system already initialized.");
        return true;
    };

    void MaterialSystem::Shutdown() {
        if (instance) {
            DEBUG("Shutting down MaterialSystem");
            delete instance;
        }
    };

    Material* MaterialSystem::AcquireMaterial(std::string name) {
        MaterialResource* resource = static_cast<MaterialResource*>(ResourceSystem::GetInstance()->LoadResource(ResourceType::MATERIAL, name));
        if (!resource) {
            ERROR("MaterialSystem::AcquireMaterial falied to load material '%s'", name.c_str());
            return nullptr;
        }
        MaterialConfig config = resource->GetConfig();
        delete resource;
        return AcquireMaterialFromConfig(config);
    };

    Material* MaterialSystem::LoadMaterial(MaterialConfig& config) {
        MaterialCreateInfo mat_create_info = {};
        mat_create_info.type = config.type;
        mat_create_info.name = config.name;
        mat_create_info.diffuse_color = config.diffuse_color;

        TextureSystem* ts = TextureSystem::GetInstance();

        if (config.name.size()) {
            mat_create_info.use = TextureUse::MAP_DIFFUSE;
            mat_create_info.texture = ts->AcquireTexture(config.diffuse_map_name, true);
            if (!mat_create_info.texture) {
                WARN("Unable to load texture '%s' for material '%s', using default.", config.diffuse_map_name.c_str(), mat_create_info.name.c_str());
                mat_create_info.texture = ts->GetDefaultTexture();
            }
        } else {
            mat_create_info.texture = nullptr;
            mat_create_info.use = TextureUse::UNKNOWN;
        }

        return RendererFrontend::GetInstance()->CreateMaterial(mat_create_info);
    };

    Material* MaterialSystem::AcquireMaterialFromConfig(MaterialConfig& config) {
        if (config.name == DEFAULT_MATERIAL_NAME) {
            return default_material;
        }

        if (registered_materials[config.name]) {
            return registered_materials[config.name];
        } else {
            registered_materials[config.name] = LoadMaterial(config);

            if (!registered_materials[config.name]) {
                ERROR("Failed to load texture '%s'.", config.name.c_str());
                return nullptr;
            }

            return registered_materials[config.name];
        }   

        return nullptr;
    };

    void MaterialSystem::ReleaseMaterial(std::string name) {
        if (registered_materials[name]) {
            delete registered_materials[name];
        }
    };

};
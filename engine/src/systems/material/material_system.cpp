#include "material_system.hpp"
#include "systems/texture/texture_system.hpp"
#include "renderer/renderer.hpp"

#include "core/logger/logger.hpp"
#include "core/utils/string.hpp"

// TODO: resource system
#include "platform/filesystem.hpp"
#include "platform/platform.hpp"
//
 
namespace Engine {

    MaterialSystem* MaterialSystem::instance = nullptr;

    MaterialSystem::MaterialSystem() {
       CreateDefaultMaterial();
    };

    MaterialSystem::~MaterialSystem() {
        DestroyDefaultMaterial();

        for (auto& [key, material] : registered_materials) { 
            delete material;
        } 

        registered_materials.clear();
    };

    void MaterialSystem::CreateDefaultMaterial() {
        MaterialCreateInfo mat_create_info = {};
        mat_create_info.name = DEFAULT_MATERIAL_NAME;
        mat_create_info.diffuse_color = glm::vec4(1, 1, 1, 1);
        mat_create_info.use = TextureUse::MAP_DIFFUSE;
        mat_create_info.texture = TextureSystem::GetInstance()->GetDefaultTexture();

        default_material = RendererFrontend::GetInstance()->CreateMaterial(mat_create_info);
    };

    void MaterialSystem::DestroyDefaultMaterial() {
        delete default_material;
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
            delete instance;
        }
    };

    Material* MaterialSystem::AcquireMaterial(std::string name) {
        std::string file_path = StringFormat("assets/materials/%s.%s", name.c_str(), "mat");

        MaterialConfig config;
        if (!LoadMaterialConfig(file_path, &config)) {
            ERROR("MaterialSystem::AcquireMaterial falied to load material '%s'", name.c_str());
            return nullptr;
        }

        return AcquireMaterialFromConfig(config);
    };

    Material* MaterialSystem::LoadMaterial(MaterialConfig& config) {
        MaterialCreateInfo mat_create_info = {};
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

    b8 MaterialSystem::LoadMaterialConfig(std::string file_path, MaterialConfig* out_config) {
        // Platform::ZMemory(out_config, sizeof(MaterialConfig));
        out_config->auto_release = false;
        out_config->diffuse_color = glm::vec4(0);
        out_config->diffuse_map_name = "";
        out_config->name = "";

        File* file = FileSystem::FileOpen(file_path, FileMode::READ, false);
        if (!file->IsReady()) {
            ERROR("Unable to open material file '%s'", file_path.c_str());
            return false;
        }
       
        std::string line;
        while (file->ReadLine(line)) {
            line = LTrim(line);
            
            if (line[0] == '#' || (!line.length())) {
                continue;
            }

            auto pair = MidString(line, '=');

            if (pair.first == "version") {
                // TODO: handle versions
            } else if (pair.first == "name") {
                out_config->name = pair.second;
            } else if (pair.first == "diffuse_color") {
                if (!Parse(pair.second, &out_config->diffuse_color)) {
                    ERROR("Error occured when parsing parameter '%s' in material file '%s'.", pair.first.c_str(), file_path.c_str());
                }
            } else if (pair.first == "diffuse_map_name") {
                out_config->diffuse_map_name = pair.second;
            }

        };

        FileSystem::FileClose(file);
        
        return true;
    };

};
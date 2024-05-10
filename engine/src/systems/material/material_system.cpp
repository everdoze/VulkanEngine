#include "material_system.hpp"
#include "systems/texture/texture_system.hpp"
#include "renderer/renderer.hpp"

#include "core/logger/logger.hpp"
#include "core/utils/string.hpp"

#include "systems/resource/resource_system.hpp"
#include "systems/shader/shader_system.hpp"

#include "resources/texture/sampler.hpp"

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

        mat_create_info.textures.push_back(
            (TextureMap){
                TextureSystem::GetInstance()->GetDefaultDiffuse(), 
                TextureUse::MAP_DIFFUSE,
                RendererFrontend::GetInstance()->CreateSampler((SamplerCreateInfo){
                    TextureFilterMode::LINEAR,
                    TextureFilterMode::LINEAR,
                    TextureRepeat::REPEAT,
                    TextureRepeat::REPEAT,
                    TextureRepeat::REPEAT
                })
            }
        );

        mat_create_info.textures.push_back(
            (TextureMap){
                TextureSystem::GetInstance()->GetDefaultSpecular(), 
                TextureUse::MAP_SPECULAR,
                RendererFrontend::GetInstance()->CreateSampler((SamplerCreateInfo){
                    TextureFilterMode::LINEAR,
                    TextureFilterMode::LINEAR,
                    TextureRepeat::REPEAT,
                    TextureRepeat::REPEAT,
                    TextureRepeat::REPEAT
                })
            }
        );

        mat_create_info.textures.push_back(
            (TextureMap){
                TextureSystem::GetInstance()->GetDefaultSpecular(), 
                TextureUse::MAP_NORMAL,
                RendererFrontend::GetInstance()->CreateSampler((SamplerCreateInfo){
                    TextureFilterMode::LINEAR,
                    TextureFilterMode::LINEAR,
                    TextureRepeat::REPEAT,
                    TextureRepeat::REPEAT,
                    TextureRepeat::REPEAT
                })
            }
        );

        mat_create_info.shader = ShaderSystem::GetInstance()->GetShader(BUILTIN_MATERIAL_SHADER_NAME);

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
            DEBUG("Shutting down MaterialSystem");
            delete instance;
        }
    };

    Material* MaterialSystem::AcquireMaterial(std::string name) {
        if (registered_materials[name]) {
            return registered_materials[name];
        }

        MaterialResource* resource = static_cast<MaterialResource*>(ResourceSystem::GetInstance()->LoadResource(ResourceType::MATERIAL, name));
        if (!resource) {
            ERROR("MaterialSystem::AcquireMaterial falied to load material '%s'", name.c_str());
            return nullptr;
        }
        MaterialConfig config = resource->GetConfig();
        delete resource;
        Material* material = LoadMaterial(config);
        if (material) {
            registered_materials[name] = material;
        }
        return material;
    };

    Material* MaterialSystem::LoadMaterial(MaterialConfig& config) {
        MaterialCreateInfo mat_create_info = {};
        mat_create_info.shader = ShaderSystem::GetInstance()->GetShader(config.shader_name);
        mat_create_info.name = config.name;
        mat_create_info.diffuse_color = config.diffuse_color;
        mat_create_info.shininess = config.shininess;

        TextureSystem* ts = TextureSystem::GetInstance();

        if (!config.name.size()) {
            ERROR("MaterialSystem::LoadMaterial - material can't be loaded without name.");
            return nullptr;
        }

        if (config.diffuse_map_name.size()) {
            Texture* diffuse;
            if (config.specular_map_name == "default") {
                diffuse = ts->GetDefaultDiffuse();
            } else {
                diffuse = ts->AcquireTexture(config.diffuse_map_name, true);
            }
            if (!diffuse) {
                WARN("Unable to load texture '%s' for material '%s', using default.", config.diffuse_map_name.c_str(), mat_create_info.name.c_str());
                diffuse = ts->GetDefaultTexture();
            }
            mat_create_info.textures.push_back(
                (TextureMap){
                    diffuse, 
                    TextureUse::MAP_DIFFUSE,
                    RendererFrontend::GetInstance()->CreateSampler((SamplerCreateInfo){
                        TextureFilterMode::LINEAR,
                        TextureFilterMode::LINEAR,
                        TextureRepeat::REPEAT,
                        TextureRepeat::REPEAT,
                        TextureRepeat::REPEAT
                    })
                }
            );
        }

        
        if (config.specular_map_name.size()) {
            Texture* specular;
            if (config.specular_map_name == "default") {
                specular = ts->GetDefaultSpecular();
            } else {
                specular = ts->AcquireTexture(config.specular_map_name, true);
            }
            if (!specular) {
                WARN("Unable to load texture '%s' for material '%s', using default.", config.specular_map_name.c_str(), mat_create_info.name.c_str());
                specular = ts->GetDefaultSpecular();
            }
            mat_create_info.textures.push_back(
                (TextureMap){
                    specular, 
                    TextureUse::MAP_SPECULAR,
                    RendererFrontend::GetInstance()->CreateSampler((SamplerCreateInfo){
                        TextureFilterMode::LINEAR,
                        TextureFilterMode::LINEAR,
                        TextureRepeat::REPEAT,
                        TextureRepeat::REPEAT,
                        TextureRepeat::REPEAT
                    })
                }
            );
        }

         if (config.normal_map_name.size()) {
            Texture* normal;
            if (config.normal_map_name == "default") {
                normal = ts->GetDefaultNormal();
            } else {
                normal = ts->AcquireTexture(config.normal_map_name, true);
            }
            if (!normal) {
                WARN("Unable to load texture '%s' for material '%s', using default.", config.normal_map_name.c_str(), mat_create_info.name.c_str());
                normal = ts->GetDefaultNormal();
            }
            mat_create_info.textures.push_back(
                (TextureMap){
                    normal, 
                    TextureUse::MAP_NORMAL,
                    RendererFrontend::GetInstance()->CreateSampler((SamplerCreateInfo){
                        TextureFilterMode::LINEAR,
                        TextureFilterMode::LINEAR,
                        TextureRepeat::REPEAT,
                        TextureRepeat::REPEAT,
                        TextureRepeat::REPEAT
                    })
                }
            );
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
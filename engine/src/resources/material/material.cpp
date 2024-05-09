#include "material.hpp"

#include "core/logger/logger.hpp"
#include "systems/shader/shader_system.hpp"
#include "platform/platform.hpp"

namespace Engine {

    Material::Material(MaterialCreateInfo& info) {
        name = info.name;
        shader = info.shader;
        id = INVALID_ID;
        generation = INVALID_ID;
        internal_id = INVALID_ID;
        current_frame = INVALID_ID;
        diffuse_color = info.diffuse_color;
        shininess = info.shininess;
        memory = nullptr;
        Platform::ZMemory(&diffuse_map, sizeof(TextureMap));
        Platform::ZMemory(&specular_map, sizeof(TextureMap));
        Platform::ZMemory(&normals_map, sizeof(TextureMap));
        for (u32 i = 0; i < info.textures.size(); ++i) {
            switch (info.textures[i].use) {
                case TextureUse::MAP_DIFFUSE:
                    diffuse_map = info.textures[i];
                    break;
                case TextureUse::MAP_SPECULAR:
                    specular_map = info.textures[i];
                    break;
                case TextureUse::MAP_NORMAL:
                    normals_map = info.textures[i];
                    break;
                default:
                    ERROR("Material::Material - unknown texture use in material '%s'.", name.c_str());
            }
        }
    };

    Material::~Material() {
        name = "";
        id = INVALID_ID;
        generation = INVALID_ID;
        internal_id = INVALID_ID;
    };

    b8 Material::AcquireInstanceResources() {
        if (!shader) {
            ERROR("Material::AcquireInstanceResources - unable to acquire resources. Shader does not exists.");
            return false;
        }

        internal_id = shader->AcquireInstanceResources();
        return true;
    };

    b8 Material::ApplyInstance(u32 frame) {
        if (!shader) {
            ERROR("Material::ApplyInstance - no shader provided in material, can't apply instance");
            return false;
        }

        if (internal_id == INVALID_ID) {
            ERROR("Material::ApplyInstance - material resources not acquired from shader.");
            return false;
        }

        b8 needs_update = current_frame != frame;
        
        shader->BindInstance(internal_id);

        if (needs_update) {
            shader->SetUniformByName("diffuse_color", &diffuse_color);

            if (diffuse_map.texture) {
                shader->SetUniformByName("diffuse_texture", diffuse_map.texture);
            }
        
            if (specular_map.texture) {
                shader->SetUniformByName("specular_texture", specular_map.texture);
            }
            if (normals_map.texture) {
                shader->SetUniformByName("normal_texture", normals_map.texture);
            }

            if (shader->GetName() == BUILTIN_MATERIAL_SHADER_NAME) {
                shader->SetUniformByName("shininess", &shininess);
            }

            current_frame = frame;
        }
        
        shader->ApplyInstance(needs_update);

        return true;
    };

    b8 Material::ApplyLocal(const glm::mat4* model) {
        if (!shader) {
            ERROR("Material::ApplyLocal - no shader provided in material, can't apply local");
            return false;
        }

        ShaderUniformConfig* uniform = shader->GetUniform("model");
        shader->SetUniform(uniform, model);
        return true;
    };

}; 
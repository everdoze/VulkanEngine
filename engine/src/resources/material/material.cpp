#include "material.hpp"

#include "core/logger/logger.hpp"
#include "systems/shader/shader_system.hpp"

namespace Engine {

    Material::Material(MaterialCreateInfo& info) {
        name = info.name;
        shader = info.shader;
        id = INVALID_ID;
        generation = INVALID_ID;
        interanal_id = INVALID_ID;
        diffuse_color = info.diffuse_color;
        diffuse_map.use = info.use;
        diffuse_map.texture = info.texture;
        memory = nullptr;
    };

    Material::~Material() {
        name = "";
        id = INVALID_ID;
        generation = INVALID_ID;
        interanal_id = INVALID_ID;
    };

    b8 Material::AcquireInstanceResources() {
        if (!shader) {
            ERROR("Material::AcquireInstanceResources - unable to acquire resources. Shader does not exists.");
            return false;
        }

        interanal_id = shader->AcquireInstanceResources();
        return true;
    };

    b8 Material::ApplyInstance() {
        if (!shader) {
            ERROR("Material::ApplyInstance - no shader provided in material, can't apply instance");
            return false;
        }

        if (interanal_id == INVALID_ID) {
            ERROR("Material::ApplyInstance - material resources not acquired from shader.");
            return false;
        }

        shader->BindInstance(interanal_id);

        shader->SetUniformByName("diffuse_color", &diffuse_color);
        shader->SetUniformByName("diffuse_texture", diffuse_map.texture);
        
        shader->ApplyInstance();

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
#include "shader_system.hpp"

#include "renderer/renderer.hpp"
#include "core/logger/logger.hpp"
#include "systems/resource/resource_system.hpp"

namespace Engine {

    ShaderSystem* ShaderSystem::instance = nullptr;

    b8 ShaderSystem::Initialize() {
        if (!instance) {
            instance = new ShaderSystem();
            return true;
        }
        ERROR("ShaderSystem is already initialized.");
        return false;
    };

    ShaderSystem::ShaderSystem() {};

    ShaderSystem::~ShaderSystem() {
        for (auto [key, map] : registered_shaders) {
            delete map.shader;
            map.ref_count = 0;
            map.auto_release = false;
        }
        registered_shaders.clear();
    };

    void ShaderSystem::Shutdown() {
        if (instance) {
            DEBUG("Shutting down ShaderSystem.");
            return delete instance;
        }
        ERROR("ShaderSystem not initialized!");
    };

    Shader* ShaderSystem::GetShader(std::string name) {
        if (!registered_shaders.count(name)) {
            ERROR("ShaderSystem::GetShader - Shader not found with name '%s'. Use ShaderSystem::CreateShader to create a shader.", name.c_str());
            return nullptr;
        }

        registered_shaders[name].ref_count++;

        return registered_shaders[name].shader;
    };

    Shader* ShaderSystem::CreateShader(std::string name, b8 auto_release) {
        ShaderResource* shader_resource = static_cast<ShaderResource*>(ResourceSystem::GetInstance()->LoadResource(ResourceType::SHADER, name));
        if (shader_resource) {
            ShaderConfig config = shader_resource->GetShaderConfig();
            return CreateShader(config, auto_release);
        }
        ERROR("ShaderSystem::CreateShader - failed to load shader '%s'", name.c_str());
        return nullptr;
    };

    b8 ShaderSystem::ApplyGlobals(std::string name, ParamsData& params) {
        Shader* shader = GetShader(name);
        if (shader) {
            shader->BindGlobals();
    
            for (u32 i = 0; i < params.size(); ++i) {
                shader->SetUniformByName(params[i].name, params[i].data);
            }
            
            shader->ApplyGlobals();

            return true;
        }
        return false;
    };

    Shader* ShaderSystem::CreateShader(ShaderConfig& config, b8 auto_release) {
        if (!config.name.size()) {
            ERROR("ShaderSystem::CreateShader - can't create shader with a blank name");
            return nullptr;
        }
        if (!config.renderpass_name.size()) {
            ERROR("ShaderSystem::CreateShader - can't create shader without renderpass_name.");
            return nullptr;
        }

        Shader* shader = RendererFrontend::GetInstance()->CreateShader(config);
        if (shader && shader->ready) {
            registered_shaders[shader->GetName()].shader = shader;
            registered_shaders[shader->GetName()].ref_count = 1;
            registered_shaders[shader->GetName()].auto_release = auto_release;
            return shader;
        } 
        delete shader;
        return nullptr;
    };

    b8 ShaderSystem::SetUniform(std::string& name, const void* value) {
        if (!current_shader) {
            ERROR("ShaderSystem::SetUniform - called without shader in use.");
            return false;
        }
        ShaderUniformConfig* uniform = current_shader->GetUniform(name);
        current_shader->SetUniform(uniform, value);
        return true;
    };

    b8 ShaderSystem::UseShader(std::string name) {
        if (!registered_shaders.count(name)) {
            ERROR("ShaderSystem::UseShader - Shader not found with name '%s'. Use ShaderSystem::CreateShader to create a shader.");
            return false;
        }

        current_shader = registered_shaders[name].shader;
        current_shader->Use();
        
        return true;
    };

    b8 ShaderSystem::DestroyShader(std::string name) {
        if (!registered_shaders.count(name)) {
            ERROR("ShaderSystem::DestroyShader - Shader not found with name '%s'. Use ShaderSystem::CreateShader to create a shader.");
            return false;
        }

        ShaderReference* shader_ref = &registered_shaders[name];

        shader_ref->ref_count--;

        if (shader_ref->auto_release && shader_ref->ref_count == 0) {
            delete shader_ref->shader;
        }
        
        return true;
    };

}   


#pragma once

#include "defines.hpp"
#include "renderer/renderer_types.inl"
#include "resources/shader/shader.hpp"

#define DEFAULT_TEXTURE_NAME "default_texture"

namespace Engine {
    // TODO: AUTO_RELEASE usage
    struct ShaderReference {
        Shader* shader;
        b8 auto_release;
        u32 ref_count;
    };

    class ShaderSystem {
        public:
            ShaderSystem();
            ~ShaderSystem();

            static b8 Initialize();
            static void Shutdown();
            static ShaderSystem* GetInstance() { return instance; };

            Shader* GetShader(std::string name);
            Shader* CreateShader(ShaderConfig& config);
            Shader* CreateShader(std::string name);
            b8 UseShader(std::string name);
            b8 DestroyShader(std::string name);
            
            b8 ApplyGlobals(std::string name, const glm::mat4* projection, const glm::mat4* view);

            b8 SetUniform(std::string& name, const void* value);

        private:
            static ShaderSystem* instance;
            std::unordered_map<std::string, Shader*> registered_shaders;

            Shader* current_shader;
    };

};
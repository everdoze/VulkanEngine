#pragma once

#include "defines.hpp"
#include "renderer/renderer_types.hpp"
#include "resources/shader/shader.hpp"

#define DEFAULT_TEXTURE_NAME "default_texture"

namespace Engine {

    struct ShaderReference {
        Shader* shader;
        b8 auto_release;
        u32 ref_count;
    };

    struct Param {
        std::string name;
        void* data;
    };

    typedef std::vector<Param> ParamsData;

    class ENGINE_API ShaderSystem {
        public:
            ShaderSystem();
            ~ShaderSystem();

            static b8 Initialize();
            static void Shutdown();
            static ShaderSystem* GetInstance() { return instance; };

            Shader* GetShader(std::string name);
            Shader* CreateShader(ShaderConfig& config, b8 auto_release = true);
            Shader* CreateShader(std::string name, b8 auto_release = true);
            b8 UseShader(std::string name);
            b8 DestroyShader(std::string name);
            
            b8 ApplyGlobals(std::string name, ParamsData& params);

            b8 SetUniform(std::string& name, const void* value);

        private:
            static ShaderSystem* instance;
            std::unordered_map<std::string, ShaderReference> registered_shaders;

            Shader* current_shader;
    };

};
#pragma once 

#include "defines.hpp"
#include "systems/resource/resources/base/resource.hpp"
#include "resources/shader/shader.hpp"

namespace Engine {

    class ENGINE_API ShaderResource: public Resource {
        public:
            ShaderResource(u32 loader_id, std::string name,
                std::string full_path, ShaderConfig config);
            ~ShaderResource();

            ShaderConfig& GetShaderConfig() { return data; };

        protected:
            ShaderConfig data;

    };

}

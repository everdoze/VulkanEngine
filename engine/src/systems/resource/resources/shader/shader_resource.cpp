#include "shader_resource.hpp"

namespace Engine {
    
    ShaderResource::ShaderResource(
        u32 loader_id, std::string name,
        std::string full_path, ShaderConfig config) : Resource(loader_id, name, full_path) {
        data = config;
    }
    
    ShaderResource::~ShaderResource() {}

}

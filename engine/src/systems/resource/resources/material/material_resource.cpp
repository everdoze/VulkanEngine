#include "material_resource.hpp"

namespace Engine {
    
    MaterialResource::MaterialResource(
        u32 loader_id, std::string name,
        std::string full_path, MaterialConfig& config) : Resource(loader_id, name, full_path) {
        data = config;
    }
    
    MaterialResource::~MaterialResource() {}
}

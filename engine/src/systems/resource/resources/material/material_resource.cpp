#include "material_resource.hpp"

namespace Engine {
    
    MaterialResource::MaterialResource(
        u32 loader_id, std::string name, 
        std::string full_path, std::string material_name, b8 auto_release,
        glm::vec4 diffuse_color, std::string diffuse_map_name
    ) : Resource(loader_id, name, full_path) {
        this->material_name = material_name;
        this->auto_release = auto_release;
        this->diffuse_color = diffuse_color;
        this->diffuse_map_name = diffuse_map_name;
    }
    
    MaterialResource::~MaterialResource() {}
}

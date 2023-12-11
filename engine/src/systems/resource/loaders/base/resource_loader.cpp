#include "resource_loader.hpp"

namespace Engine {
    
    ResourceLoader::ResourceLoader(u32 id, ResourceType type, std::string type_path, std::string custom_type) {
        this->id = id;
        this->type = type;
        this->type_path = type_path;
        this->custom_type = custom_type;
    };

    ResourceLoader::ResourceLoader(u32 id, ResourceType type, std::string type_path) {
        this->id = id;
        this->type = type;
        this->type_path = type_path;
    };

    ResourceLoader::~ResourceLoader() {

    };

} 

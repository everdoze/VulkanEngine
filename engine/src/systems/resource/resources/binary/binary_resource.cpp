#include "binary_resource.hpp"

namespace Engine {
    BinaryResource::BinaryResource(
        u32 loader_id, std::string name, 
        std::string full_path, std::vector<c8> data
    ) : Resource(loader_id, name, full_path) {
        this->data = data;
    };

    BinaryResource::~BinaryResource() {};
}
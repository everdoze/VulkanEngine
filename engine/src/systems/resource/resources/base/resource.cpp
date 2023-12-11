#include "resource.hpp"

namespace Engine {

    Resource::Resource(u32 loader_id, std::string name, std::string full_path) {
        this->loader_id = loader_id;
        this->name = name;
        this->full_path = full_path;
    };

    Resource::~Resource() {

    };

}

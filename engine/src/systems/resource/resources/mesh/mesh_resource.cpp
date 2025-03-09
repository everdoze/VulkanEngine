#include "mesh_resource.hpp"

#include "platform/platform.hpp"

namespace Engine {

    MeshResource::MeshResource(
        u32 loader_id, std::string name,
        std::string full_path, GeometryConfigs configs) : Resource(loader_id, name, full_path) {
        this->configs = configs;
    };

    MeshResource::~MeshResource() {
        for (auto& config: configs) {
            Platform::FrMemory(config.indices);
            Platform::FrMemory(config.vertices);
        }
    };

}
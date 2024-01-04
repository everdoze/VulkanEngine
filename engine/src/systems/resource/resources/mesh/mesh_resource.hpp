#pragma once

#include "defines.hpp"
#include "systems/resource/resources/base/resource.hpp"

namespace Engine {
    
    struct GeometryExtent {
        glm::vec3 center;
        glm::vec3 min_extents;
        glm::vec3 max_extents;
    };

    struct GeometryConfig {
        u32 vertex_size;
        u32 vertex_count;
        void* vertices;
        u32 index_size;
        u32 index_count;
        GeometryExtent extent;
        void* indices;
        std::string name;
        std::string material_name;
    };

    typedef std::vector<GeometryConfig> GeometryConfigs;

    class MeshResource : public Resource {
        public:
            MeshResource(
                u32 loader_id, std::string name,
                std::string full_path, GeometryConfigs configs
            );
            ~MeshResource();

            GeometryConfigs& GetConfigs() { return configs; };
            
        protected:
            GeometryConfigs configs;
    };

}
#pragma once

#include "defines.hpp"

#include "resources/geometry/geometry.hpp"
#include "math/transform/transform.hpp"

namespace Engine {
    
    struct ENGINE_API MeshCreateConfig {
        Transform* transform;
        std::vector<Geometry*> geometries;
    };

    class ENGINE_API Mesh {
        public:
            Mesh(MeshCreateConfig config);
            virtual ~Mesh();

            std::vector<Geometry*> geometries;
            Transform* transform;
    };

} 

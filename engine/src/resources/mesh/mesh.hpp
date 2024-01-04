#pragma once

#include "defines.hpp"

#include "resources/geometry/geometry.hpp"
#include "math/transform/transform.hpp"

namespace Engine {
    
    struct MeshCreateConfig {
        Transform* transform;
        std::vector<Geometry*> geometries;
    };

    class Mesh {
        public:
            Mesh(MeshCreateConfig config);
            virtual ~Mesh();

            std::vector<Geometry*> geometries;
            Transform* transform;
    };

} 

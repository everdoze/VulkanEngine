#include "mesh.hpp"

namespace Engine {


    Mesh::Mesh(MeshCreateConfig config) {
        geometries = config.geometries;
        transform = config.transform;
        if (!transform) {
            transform = new Transform();
        }
    };

    Mesh::~Mesh() {
        geometries.clear();
        delete transform;
    };

}
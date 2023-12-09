#include "geometry.hpp"

namespace Engine {

    Geometry::Geometry(GeometryCreateInfo& info) {
        id = info.id;
        interanal_id = info.id;
        generation = INVALID_ID;
        name = info.name;
        material = info.material;
    }

    Geometry::~Geometry() {

    }

}
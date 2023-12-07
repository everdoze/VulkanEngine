#include "material.hpp"

namespace Engine {

    Material::Material(MaterialCreateInfo& info) {
        name = info.name;
        id = INVALID_ID;
        generation = INVALID_ID;
        interanal_id = INVALID_ID;
        diffuse_color = info.diffuse_color;
        diffuse_map.use = info.use;
        diffuse_map.texture = info.texture;
    };

    Material::~Material() {
        name = "";
        id = INVALID_ID;
        generation = INVALID_ID;
        interanal_id = INVALID_ID;
    };

};
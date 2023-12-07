#include "texture.hpp"

namespace Engine {

    Texture::Texture(TextureCreateInfo& info) {
        this->name = info.name;
        this->width = info.width;
        this->height = info.height;
        this->channel_count = info.channel_count;
        this->has_transparency = info.has_transparency;
        this->generation = INVALID_ID;
        this->id = INVALID_ID;
    };

    Texture::~Texture() {
        this->name = "";
        this->width = 0;
        this->height = 0;
        this->channel_count = 0;
        this->has_transparency = false;
        this->generation = INVALID_ID;
        this->id = INVALID_ID;
    };

};
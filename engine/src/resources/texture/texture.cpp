#include "texture.hpp"

namespace Engine {

    Texture::Texture(TextureCreateInfo& info) {
        this->name = info.name;
        this->width = info.width;
        this->height = info.height;
        this->flags = info.flags;
        this->channel_count = info.channel_count;
        this->generation = INVALID_ID;
        this->id = INVALID_ID;
    };

    Texture::~Texture() {
        this->name = "";
        this->width = 0;
        this->height = 0;
        this->channel_count = 0;
        this->flags = TextureFlag::NONE;
        this->generation = INVALID_ID;
        this->id = INVALID_ID;
    };
};
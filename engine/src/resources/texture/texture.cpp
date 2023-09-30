#include "texture.hpp"

namespace Engine {

    Texture::Texture(
        std::string name,
        u32 width,
        u32 height,
        u8 channel_count,
        u8 has_transparency,
        u8* pixels) {
        
        this->name = name;
        this->width = width;
        this->channel_count = channel_count;
        this->has_transparency = has_transparency;
        this->generation = INVALID_ID;
        this->id = INVALID_ID;
    };

    Texture::~Texture() {
        this->name = "";
        this->width = 0;
        this->channel_count = 0;
        this->has_transparency = false;
        this->generation = INVALID_ID;
        this->id = INVALID_ID;
    };

};
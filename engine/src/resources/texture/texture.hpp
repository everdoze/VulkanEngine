#pragma once

#include "defines.hpp"

#include "core/utils/freelist.hpp"

namespace Engine {

    enum class TextureUse {
        UNKNOWN = 0x00,
        MAP_DIFFUSE = 0x01,
        MAP_SPECULAR = 0x02,
        MAP_NORMAL = 0x04
    };

    struct TextureCreateInfo {
        public:
            std::string name;
            u32 width;
            u32 height;
            u8 channel_count;
            u8 has_transparency;
            u8* pixels;
    };

    class Texture {
        public:
            Texture(TextureCreateInfo& info);
            virtual ~Texture();

            u32 GetId() { return id; };
            u32 GetWidth() { return width; };
            u32 GetHeight() { return height; };

            u8 GetChannelCount() { return channel_count; };
            u32 HasTransparency() { return has_transparency; };
            u32 GetGeneration() { return generation; };
            void SetGeneration(u32 generation) { this->generation = generation; };
            void UpdateGeneration () { 
                if (generation == INVALID_ID) {
                    generation = 0;
                } else {
                    generation++;
                }
            };

        protected:
            std::string name;
            u32 id;
            u32 internal_id;
            u32 width;
            u32 height;
            u8 channel_count;
            b8 has_transparency;
            u32 generation;
    };


};
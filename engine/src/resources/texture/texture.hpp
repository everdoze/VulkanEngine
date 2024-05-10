#pragma once

#include "defines.hpp"

#include "core/utils/freelist.hpp"
#include "sampler.hpp"
#include "texture_types.hpp"

namespace Engine {

    struct TextureCreateInfo {
        std::string name;
        u32 width;
        u32 height;
        u8 channel_count;
        b8 has_transparency;
        b8 is_writeable;
        u8* pixels;
    };

    class Texture {
        public:
            Texture(TextureCreateInfo& info);
            virtual ~Texture();

            u32 GetId() { return id; };

            u32 GetWidth() { return width; };

            u32 GetHeight() { return height; };
            
            std::string& GetName() { return name; };

            u8 GetChannelCount() { return channel_count; };

            b8 HasTransparency() { return has_transparency; };

            b8 IsWriteable() { return is_writeable; };

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
            b8 is_writeable;
            u32 generation;
    };

    struct TextureMap {
        Texture* texture;
        TextureUse use;
        Sampler* sampler;

        // TextureFilterMode filter_minify;
        // TextureFilterMode filter_magnify;

        // TextureRepeat repeat_u;
        // TextureRepeat repeat_v;
        // TextureRepeat repeat_w;
    };

};
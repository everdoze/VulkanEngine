#pragma once

#include "defines.hpp"

#include "core/utils/freelist.hpp"
#include "core/logger/logger.hpp"
#include "sampler.hpp"
#include "texture_types.hpp"

namespace Engine {

    enum class TextureFlag {
        NONE = 0x00,
        HAS_TRANSPARENCY = 0x01,
        IS_WRITEABLE = 0x02,
        IS_WRAPPED = 0x04
    };

    ENABLE_BITMASK_OPERATORS(TextureFlag)

    struct TextureCreateInfo {
        std::string name;
        u32 width;
        u32 height;
        u8 channel_count;
        TextureFlag flags;
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

            b8 HasTransparency() { return flags & TextureFlag::HAS_TRANSPARENCY; };

            b8 IsWriteable() { return flags & TextureFlag::IS_WRITEABLE; };

            u32 GetGeneration() { return generation; };

            void SetGeneration(u32 generation) { this->generation = generation; };

            void UpdateGeneration () { 
                if (generation == INVALID_ID) {
                    generation = 0;
                } else {
                    generation++;
                }
            };

            virtual void WriteData(const u8* pixels, u32 offset = 0, u32 size = 0) {
                WARN("Texture::WriteData is not implemented");
            };

            virtual void Resize(u32 width, u32 height) {
                WARN("Texture::Resize is not implemented");
            };

        protected:
            std::string name;
            u32 id;
            u32 internal_id;
            u32 width;
            u32 height;
            u8 channel_count;
            TextureFlag flags;
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
#pragma once

#include "defines.hpp"
#include "renderer/renderer_types.hpp"
#include "resources/texture/texture.hpp"

#define DEFAULT_TEXTURE_NAME "default_texture"
#define DEFAULT_SPECULAR_NAME "default_specular"
#define DEFAULT_NORMAL_NAME "default_normal"

namespace Engine {

    struct TextureReference {
        Texture* texture;
        b8 auto_release;
        u32 ref_count;
    };

    class ENGINE_API TextureSystem {
        public:
            TextureSystem();
            ~TextureSystem();

            static b8 Initialize();
            static void Shutdown();
            static TextureSystem* GetInstance();

            Texture* AcquireWriteableTexture(std::string name, u32 width, u32 height, u8 channel_count, b8 has_transparency, b8 register_texture);
            Texture* AcquireTexture(std::string name, b8 auto_release = true);
            void ReleaseTexture(std::string name);

            b8 CreateDefaultTextures();
            void DestroyDefaultTextures();

            Texture* GetDefaultTexture() { return default_texture; };
            Texture* GetDefaultDiffuse() { return default_diffuse; };
            Texture* GetDefaultSpecular() { return default_specular; };
            Texture* GetDefaultNormal() { return default_normal; };

            Texture* LoadTexture(std::string texture_name);
        
        private:
            static TextureSystem* instance;
            Texture* default_texture;
            Texture* default_diffuse;
            Texture* default_specular;
            Texture* default_normal;
            std::unordered_map<std::string, TextureReference> registered_textures;
    };

};
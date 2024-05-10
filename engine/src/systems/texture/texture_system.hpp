#pragma once

#include "defines.hpp"
#include "renderer/renderer_types.hpp"
#include "resources/texture/texture.hpp"

#define DEFAULT_TEXTURE_NAME "default_texture"
#define DEFAULT_SPECULAR_NAME "default_specular"
#define DEFAULT_NORMAL_NAME "default_normal"

namespace Engine {
    // TODO: AUTO_RELEASE usage
    struct TextureReference {
        Texture* texture;
        b8 auto_release;
        u32 ref_count;
    };

    class TextureSystem {
        public:
            TextureSystem();
            ~TextureSystem();

            static b8 Initialize();
            static void Shutdown();
            static TextureSystem* GetInstance();

            Texture* AcquireTexture(std::string name, b8 auto_release);
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
            std::unordered_map<std::string, Texture*> registered_textures;
    };

};
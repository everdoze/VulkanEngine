#pragma once

#include "defines.hpp"
#include "renderer/renderer_types.inl"

#define DEFAULT_TEXTURE_NAME "default_texture"

namespace Engine {

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

            Texture* LoadTexture(std::string texture_name);
        
        private:
            static TextureSystem* instance;
            Texture* default_texture;
            std::unordered_map<std::string, Texture*> registered_textures;
    };

};
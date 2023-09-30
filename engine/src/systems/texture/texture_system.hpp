#pragma once

#include "defines.hpp"
#include "renderer/renderer_types.inl"

#define DEFAULT_TEXTURE_NAME "default_texture"

namespace Engine {

    // struct RegisteredTexture {
    //     u32 reference_count = 0;
    //     Ref<Texture> handle = nullptr;
    //     b8 auto_release = false;

    //     operator bool() const {
    //         return !!handle;
    //     }
    // };

    class TextureSystem {
        public:
            TextureSystem();
            ~TextureSystem();

            static b8 Initialize();
            static void Shutdown();
            static Ref<TextureSystem> GetInstance();

            Ref<Texture> AcquireTexture(std::string name, b8 auto_release);
            void ReleaseTexture(std::string name);

            b8 CreateDefaultTextures();
            void DestroyDefaultTextures();

            Ref<Texture> GetDefaultTexture() { return default_texture; };

            Ref<Texture> LoadTexture(std::string texture_name);
        
        private:
            static Ref<TextureSystem> instance;
            Ref<Texture> default_texture;
            std::unordered_map<std::string, Ref<Texture>> registered_textures;
    };

};
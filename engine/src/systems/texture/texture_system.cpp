#include "texture_system.hpp"

#include "renderer/renderer.hpp"
#include "core/logger/logger.hpp"
#include "platform/platform.hpp"

// TODO: Resource loader
#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image/stb_image.h"

namespace Engine {
    
    Ref<TextureSystem> TextureSystem::instance = nullptr;

    TextureSystem::TextureSystem() {
        CreateDefaultTextures();
    };

    TextureSystem::~TextureSystem() {
        DestroyDefaultTextures();
        registered_textures.clear();
        INFO("%i", registered_textures.size());
    };

    b8 TextureSystem::Initialize() {
        if (!instance) {
            instance = CreateRef<TextureSystem>();
        }
        return true;
    };

    void TextureSystem::Shutdown() {
        instance = nullptr;
    };

    Ref<TextureSystem> TextureSystem::GetInstance() {
        if (instance) {
            return instance;
        }
        ERROR("Probably TextureSystem::GetInstance() called before initialization.");
        return nullptr;
    };

    Ref<Texture> TextureSystem::AcquireTexture(std::string name, b8 auto_release) {
        if (name == DEFAULT_TEXTURE_NAME) {
            WARN("TextureSystem::AcquireTexture called for default texture.");
            return default_texture;
        }

        if (registered_textures[name]) {
            return registered_textures[name];
        } else {
            registered_textures[name] = LoadTexture(name);

            if (!registered_textures[name]) {
                ERROR("Failed to load texture '%s'.", name.c_str());
                return nullptr;
            }

            return registered_textures[name];
        }   

        return nullptr;
    };

    void TextureSystem::ReleaseTexture(std::string name) {
        if (name == DEFAULT_TEXTURE_NAME) {
            WARN("TextureSystem::ReleaseTexture called for default texture.");
            return;
        }

        if (registered_textures[name]) {
            registered_textures.erase(name);
            return;
        }

        ERROR("TextureSystem::ReleaseTexture called for non-existing texture.");
    };

    b8 TextureSystem::CreateDefaultTextures() {
        const u32 tex_dimension = 256;
        const u32 channels = 4;
        const u32 pixel_count = tex_dimension * tex_dimension;
        u8 pixels[pixel_count * channels];

        Platform::SMemory(pixels, 255, sizeof(u8) * pixel_count * channels);

        // Each pixel.
        for (u64 row = 0; row < tex_dimension; ++row) {
            for (u64 col = 0; col < tex_dimension; ++col) {
                u64 index = (row * tex_dimension) + col;
                u64 index_bpp = index * channels;
                if (row % 2) {
                    if (col % 2) {
                        pixels[index_bpp + 0] = 0;
                        pixels[index_bpp + 1] = 0;
                    }
                } else {
                    if (!(col % 2)) {
                        pixels[index_bpp + 0] = 0;
                        pixels[index_bpp + 1] = 0;
                    }
                }
            }
        }

        default_texture = RendererFrontend::GetInstance()->CreateTexture(
            DEFAULT_TEXTURE_NAME,
            tex_dimension, tex_dimension,
            channels, false,
            pixels);

        return true;
    };

    void TextureSystem::DestroyDefaultTextures() {
        default_texture = nullptr;
    };

    Ref<Texture> TextureSystem::LoadTexture(std::string texture_name) {
        std::string file_path = StringFormat("assets/textures/%s.%s", texture_name.c_str(), "png");
        stbi_set_flip_vertically_on_load(true);

        const u8 required_channel_count = 4;

        i32 width;
        i32 height;
        i32 channel_count;
        b8 has_transparency = false;
        u8* data = stbi_load(
            file_path.c_str(),
            &width, &height,
            &channel_count,
            required_channel_count);


        Ref<Texture> texture = nullptr;

        if (data) {
            u64 total_size = width * height * required_channel_count;
            for (u64 i = 0; i < total_size; i += required_channel_count) {
                u8 alpha = data[i + 3];
                if (alpha < 255) {
                    has_transparency = true;
                    break;
                }
            }

            if (stbi_failure_reason()) {
                WARN("RendererFrontend::LoadTexture failed to load file '%s' : %s", file_path.c_str(), stbi_failure_reason());
                stbi__err(0, 0);
            }

            texture = RendererFrontend::GetInstance()->CreateTexture(
                texture_name, width, height,
                required_channel_count, has_transparency,
                data
            );

            texture->UpdateGeneration();

            stbi_image_free(data);
        } else {
            if (stbi_failure_reason()) {
                WARN("RendererFrontend::LoadTexture failed to load file '%s' : %s", file_path.c_str(), stbi_failure_reason());
                stbi__err(0, 0);
            }
        }

        return texture;
    };

};
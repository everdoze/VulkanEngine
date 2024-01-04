#include "texture_system.hpp"

#include "renderer/renderer.hpp"
#include "core/logger/logger.hpp"
#include "platform/platform.hpp"

#include "systems/resource/resource_system.hpp"

namespace Engine {
    
    TextureSystem* TextureSystem::instance = nullptr;

    TextureSystem::TextureSystem() {
        CreateDefaultTextures();
    };

    TextureSystem::~TextureSystem() {
        DestroyDefaultTextures();

        for (auto& [key, texture] : registered_textures) { 
            delete texture;
        } 

        registered_textures.clear();
    };

    b8 TextureSystem::Initialize() {
        if (!instance) {
            instance = new TextureSystem();
        }
        return true;
    };

    void TextureSystem::Shutdown() {
        if (instance) {
            DEBUG("Shutting down TextureSystem");
            delete instance;
        }
    };

    TextureSystem* TextureSystem::GetInstance() {
        if (instance) {
            return instance;
        }
        ERROR("Probably TextureSystem::GetInstance() called before initialization.");
        return nullptr;
    };

    Texture* TextureSystem::AcquireTexture(std::string name, b8 auto_release) {
        if (name == DEFAULT_TEXTURE_NAME) {
            WARN("TextureSystem::AcquireTexture called for default texture.");
            return default_diffuse;
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
            delete registered_textures[name];
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

        TextureCreateInfo create_info;
        create_info.name = DEFAULT_TEXTURE_NAME;
        create_info.width = tex_dimension;
        create_info.height = tex_dimension;
        create_info.channel_count = channels;
        create_info.has_transparency = false;
        create_info.pixels = pixels;
        default_diffuse = RendererFrontend::GetInstance()->CreateTexture(create_info);

        u8 spec_pixels[16 * 16 * 4];
        // Default specular map is black
        Platform::ZMemory(spec_pixels, sizeof(u8) * 16 * 16 * 4);
        TextureCreateInfo spec_create_info;
        spec_create_info.name = DEFAULT_SPECULAR_NAME;
        spec_create_info.width = 16;
        spec_create_info.height = 16;
        spec_create_info.channel_count = 4;
        spec_create_info.has_transparency = false;
        spec_create_info.pixels = spec_pixels;
        default_specular = RendererFrontend::GetInstance()->CreateTexture(spec_create_info);

        // Normal texture.
        u8 normal_pixels[16 * 16 * 4];  // w * h * channels
        Platform::ZMemory(normal_pixels, sizeof(u8) * 16 * 16 * 4);

        // Each pixel.
        for (u64 row = 0; row < 16; ++row) {
            for (u64 col = 0; col < 16; ++col) {
                u64 index = (row * 16) + col;
                u64 index_bpp = index * channels;
                // Set blue, z-axis by default and alpha.
                normal_pixels[index_bpp + 0] = 128;
                normal_pixels[index_bpp + 1] = 128;
                normal_pixels[index_bpp + 2] = 255;
                normal_pixels[index_bpp + 3] = 255;
            }
        }

        TextureCreateInfo norm_create_info;
        norm_create_info.name = DEFAULT_NORMAL_NAME;
        norm_create_info.width = 16;
        norm_create_info.height = 16;
        norm_create_info.channel_count = 4;
        norm_create_info.has_transparency = false;
        norm_create_info.pixels = normal_pixels;
        default_normal = RendererFrontend::GetInstance()->CreateTexture(norm_create_info);

        return true;
    };

    void TextureSystem::DestroyDefaultTextures() {
        delete default_diffuse;
        delete default_specular;
        delete default_normal;
    };

    Texture* TextureSystem::LoadTexture(std::string texture_name) {
        ResourceSystem* rs = ResourceSystem::GetInstance();

        ImageResource* image = static_cast<ImageResource*>(rs->LoadResource(ResourceType::IMAGE, texture_name));

        Texture* texture = nullptr;

        if (image->GetPixels()) {
            TextureCreateInfo create_info;
            create_info.name = texture_name;
            create_info.width = image->GetWidth();
            create_info.height = image->GetHeight();
            create_info.channel_count = image->GetChannelCount();
            create_info.has_transparency = image->HasTransparency();
            create_info.pixels = image->GetPixels();
            texture = RendererFrontend::GetInstance()->CreateTexture(create_info);

            texture->UpdateGeneration();
        }

        delete image;

        return texture;
    };

};
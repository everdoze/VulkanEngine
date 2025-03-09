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

        for (auto& [key, map] : registered_textures) { 
            delete map.texture;
            map.ref_count = 0;
            map.auto_release = false;
        } 

        registered_textures.clear();
    };

    b8 TextureSystem::Initialize() {
        if (!instance) {
            instance = new TextureSystem();
            return true;
        }
        DEBUG("TextureSystem is already initialized.");
        return true;
    };

    void TextureSystem::Shutdown() {
        if (instance) {
            DEBUG("Shutting down TextureSystem.");
            return delete instance;
        }
        ERROR("TextureSystem is not initialized.");
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

        if (registered_textures.count(name)) {
            registered_textures[name].ref_count++;
            return registered_textures[name].texture;
        } else {
            registered_textures[name].texture = LoadTexture(name);
            registered_textures[name].auto_release = auto_release;
            registered_textures[name].ref_count = 1;

            if (!registered_textures[name].texture) {
                ERROR("Failed to load texture '%s'.", name.c_str());
                return nullptr;
            }

            return registered_textures[name].texture;
        }   

        return nullptr;
    };

    void TextureSystem::ReleaseTexture(std::string name) {
        if (name == DEFAULT_TEXTURE_NAME) {
            WARN("TextureSystem::ReleaseTexture called for default texture.");
            return;
        }

        if (registered_textures.count(name)) {
            registered_textures[name].ref_count--;
            if (registered_textures[name].ref_count == 0 && registered_textures[name].auto_release) {
                delete registered_textures[name].texture;
                registered_textures.erase(name);
            }
            return;
        }

        ERROR("TextureSystem::ReleaseTexture called for non-existing texture.");
    };

    b8 TextureSystem::CreateDefaultTextures() {
        // Default texture
        const u32 tex_dimension = 256;
        const u32 channels = 4;
        const u32 pixel_count = tex_dimension * tex_dimension;
        u8 pixels[pixel_count * channels];

        Platform::SetMemory(pixels, 255, sizeof(u8) * pixel_count * channels);

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
        create_info.flags = TextureFlag::NONE;
        create_info.pixels = pixels;
        default_texture = RendererFrontend::GetInstance()->CreateTexture(create_info);
        ///////////////////////////////////////////////////

        // Default diffuse texture is white
        u8 diff_pixels[16 * 16 * 4];
        Platform::SetMemory(pixels, 255, sizeof(u8) * 16 * 16 * 4);

        TextureCreateInfo diff_create_info;
        diff_create_info.name = DEFAULT_TEXTURE_NAME;
        diff_create_info.width = 16;
        diff_create_info.height = 16;
        diff_create_info.channel_count = channels;
        diff_create_info.flags = TextureFlag::NONE;
        diff_create_info.pixels = pixels;
        default_diffuse = RendererFrontend::GetInstance()->CreateTexture(diff_create_info);
        ////////////////////////////////////////

        // Default specular map is black
        u8 spec_pixels[16 * 16 * 4];
        Platform::ZrMemory(spec_pixels, sizeof(u8) * 16 * 16 * 4);

        TextureCreateInfo spec_create_info;
        spec_create_info.name = DEFAULT_SPECULAR_NAME;
        spec_create_info.width = 16;
        spec_create_info.height = 16;
        spec_create_info.channel_count = 4;
        spec_create_info.flags = TextureFlag::NONE;
        spec_create_info.pixels = spec_pixels;
        default_specular = RendererFrontend::GetInstance()->CreateTexture(spec_create_info);
        //////////////////////////////////////////

        // Default normal texture is blue
        u8 normal_pixels[16 * 16 * 4];  // w * h * channels
        Platform::SetMemory(normal_pixels, 255, sizeof(u8) * 16 * 16 * 4);

        for (u64 row = 0; row < 16; ++row) {
            for (u64 col = 0; col < 16; ++col) {
                u64 index = (row * 16) + col;
                u64 index_bpp = index * channels;
                // Set blue, z-axis by default and alpha.
                normal_pixels[index_bpp + 0] = 128;
                normal_pixels[index_bpp + 1] = 128;
            }
        }

        TextureCreateInfo norm_create_info;
        norm_create_info.name = DEFAULT_NORMAL_NAME;
        norm_create_info.width = 16;
        norm_create_info.height = 16;
        norm_create_info.channel_count = 4;
        norm_create_info.flags = TextureFlag::NONE;
        norm_create_info.pixels = normal_pixels;
        default_normal = RendererFrontend::GetInstance()->CreateTexture(norm_create_info);
        ///////////////////////////////////////////

        return true;
    };

    void TextureSystem::DestroyDefaultTextures() {
        delete default_texture;
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
            create_info.flags = image->HasTransparency() ? TextureFlag::HAS_TRANSPARENCY : TextureFlag::NONE;
            create_info.pixels = image->GetPixels();
            texture = RendererFrontend::GetInstance()->CreateTexture(create_info);

            texture->UpdateGeneration();
        }

        delete image;

        return texture;
    };

    Texture* TextureSystem::AcquireWriteableTexture(std::string name, u32 width, u32 height, u8 channel_count, b8 has_transparency, b8 register_texture) {
        TextureCreateInfo create_info;
        create_info.name = name;
        create_info.width = width;
        create_info.height = height;
        create_info.channel_count = channel_count;
        create_info.flags |= has_transparency ? TextureFlag::HAS_TRANSPARENCY : TextureFlag:: NONE;
        create_info.flags |= TextureFlag::IS_WRITEABLE;

        return RendererFrontend::GetInstance()->CreateTexture(create_info);
    };

};
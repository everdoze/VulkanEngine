#include "shader.hpp"

#include <vulkan/vulkan.h>
#include "core/logger/logger.hpp"
#include "systems/texture/texture_system.hpp"
#include "platform/platform.hpp"
#include "renderer/renderer.hpp"
#include "resources/texture/sampler.hpp"

namespace Engine {

    Shader::Shader(ShaderConfig& config) {
        name = config.name;
        state = ShaderState::CREATED;
        use_instances = config.use_instances;
        use_locals = config.use_local;
        uniforms = config.uniforms;
        stages = config.stages;
        ubo_stride = config.ubo_stride;

        Platform::ZMemory(push_constant_ranges, sizeof(MemoryRange) * 32);

        // This is hard-coded because the Vulkan spec only guarantees that a _minimum_ 128 bytes of space are available,
        // and it's up to the driver to determine how much is available. Therefore, to avoid complexity, only the
        // lowest common denominator of 128B will be used.
        ubo = {};
        global_ubo = {};
        ubo_stride = 0;
        global_ubo_stride = 0;
        push_constant_stride = 128;
        push_constant_size = 0;
        push_constant_count = 0;
        attribute_stride = 0;
        required_ubo_alignment = 0;
        instance_texture_count = 0;

        bound_scope = ShaderScope::GLOBAL;
        bound_instance_id = 0;
        bound_ubo_offset = 0;

        // Process uniforms
        for (u32 i = 0; i < uniforms.size(); ++i) {
            ShaderUniformConfig* uniform = &uniforms[i];
            b8 is_sampler = uniform->type == ShaderUniformType::SAMPLER;
            b8 is_global = uniform->scope == ShaderScope::GLOBAL;

            uniform->id = i;
            uniforms_lookup[uniform->name] = &uniforms[i];

            if (is_sampler) {
                uniform->offset = 0;
                if (is_global) {
                    uniform->location = global_texture_maps.size();
                    global_texture_maps.push_back(
                        (TextureMap){
                            TextureSystem::GetInstance()->GetDefaultTexture(),
                            TextureUse::UNKNOWN,
                            RendererFrontend::GetInstance()->CreateSampler((SamplerCreateInfo){
                                TextureFilterMode::LINEAR,
                                TextureFilterMode::LINEAR,
                                TextureRepeat::REPEAT,
                                TextureRepeat::REPEAT,
                                TextureRepeat::REPEAT
                            })
                        }
                    );
                } else {
                    uniform->location = instance_texture_count;
                    instance_texture_count++;
                }
            } else {
                uniform->location = i;
                uniform->offset = ubo.size;
            }
            
            if (uniform->scope == ShaderScope::LOCAL) {
                if (uniform->scope == ShaderScope::LOCAL && !use_locals) {
                    // TODO: обработка нормальная, если не создался
                    ERROR("Shader::Shader - Error during creation shader '%s'. Cannot add local uniform to shader that doesn't use locals.", config.name.c_str());
                    continue;
                }
                
                MemoryRange range = GetAlignedMemory(push_constant_size, uniform->size, 4);
                uniform->offset = range.offset;
                uniform->size = range.size;

                push_constant_ranges[push_constant_count] = range;
                push_constant_count++;

                push_constant_size+=uniform->size;
            }

            

            if (!is_sampler) {
                if (is_global) {
                    uniform->offset = global_ubo.size;
                    global_ubo.size += uniform->size;
                } else {
                    ubo.size += uniform->size;
                }
            }
        }

        ubo.offset = global_ubo.size;
    }

    Shader::~Shader() {
        if (global_texture_maps.size()) {
            DEBUG("Destroying global samplers for shader '%s'.", name.c_str());
            for (TextureMap& map : global_texture_maps) {
                delete map.sampler;
            }
            global_texture_maps.clear();
        }
    };

    ShaderUniformConfig* Shader::GetUniform(std::string name) {
        return uniforms_lookup[name];
    };

    b8 Shader::SetUniformByName(std::string name, const void* value) {
        ShaderUniformConfig* uniform = this->GetUniform(name);
        this->SetUniform(uniform, value);
        return true;
    };

}
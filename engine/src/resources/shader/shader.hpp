#pragma once

#include "defines.hpp"

#include "resources/texture/texture.hpp"

namespace Engine {

    #define BUILTIN_MATERIAL_SHADER_NAME "Builtin.MaterialShader"
    #define BUILTIN_UI_SHADER_NAME "Builtin.UIShader"

    enum class ShaderScope {
        GLOBAL,
        INSTANCE,
        LOCAL,
        LENGTH
    };

    enum class ShaderState { 
        CREATED = 0x01,
        INITIALIZED = 0x2
    };
    
    enum class ShaderAttributeType {
        UNKNOWN,
        FLOAT32,
        FLOAT32_2,
        FLOAT32_3,
        FLOAT32_4,
        MATRIX_4x4,
        INT8,
        INT16,
        INT32,
        UINT8,
        UINT16,
        UINT32,
        LENGTH
    };

    enum class ShaderUniformType {
        UNKNOWN,
        FLOAT32,
        FLOAT32_2,
        FLOAT32_3,
        FLOAT32_4,
        MATRIX_4x4,
        INT8,
        INT16,
        INT32,
        UINT8,
        UINT16,
        UINT32,
        SAMPLER,
        CUSTOM = 255
    };

    enum class ShaderStage {
        VERTEX = 0x00000001,
        GEOMETRY = 0x00000002,
        FRAGMENT = 0x00000004,
        COMPUTE = 0x00000008
    };  

    struct ShaderStageConfig {
        std::string name;
        std::string file_path;
        ShaderStage stage;
    };

    struct ShaderAttrConfig {
        std::string name;
        u8 size;
        ShaderAttributeType type;
    };

    struct ShaderUniformConfig {
        std::string name;
        u32 id;
        u32 location;
        u8 size;
        u64 offset;
        ShaderUniformType type;
        ShaderScope scope;
    };

    struct ShaderConfig {
        std::string name;
        b8 use_instances;
        b8 use_local;
        u64 ubo_stride;
        std::vector<ShaderAttrConfig> attributes;
        std::vector<ShaderUniformConfig> uniforms;
        std::string renderpass_name;
        std::vector<ShaderStageConfig> stages;
    };


    class Shader {
        public:
            Shader(ShaderConfig& config);
            virtual ~Shader();

            virtual b8 SetUniform(ShaderUniformConfig* uniform, const void* value) = 0;
            b8 SetUniformByName(std::string name, const void* value);
            virtual ShaderUniformConfig* GetUniform(std::string name);
            std::string& GetName() { return name; };

            virtual void Use() = 0;
            virtual void BindGlobals() = 0;
            virtual void BindInstance(u32 instance_id) = 0;

            virtual void ApplyGlobals() = 0;
            virtual void ApplyInstance(b8 needs_update) = 0;

            virtual u32 AcquireInstanceResources(std::vector<TextureMap*> texture_maps) = 0;
            virtual void ReleaseInstanceResources(u32 instance_id) = 0;

            b8 ready;
            
        protected:
            std::string name;
            b8 use_instances;
            b8 use_locals;
            
            std::vector<ShaderUniformConfig> uniforms;
            std::unordered_map<std::string, ShaderUniformConfig*> uniforms_lookup;
            std::vector<ShaderStageConfig> stages;
        
            u64 required_ubo_alignment;

            FreelistNode* global_ubo_memory;
            MemoryRange global_ubo;
            u64 global_ubo_stride;

            MemoryRange ubo;
            u64 ubo_stride;

            u64 push_constant_size;
            u64 push_constant_stride;
            u32 push_constant_count;
            MemoryRange push_constant_ranges[32];

            std::vector<TextureMap> global_texture_maps;
            u8 instance_texture_count;

            ShaderScope bound_scope;
            u32 bound_instance_id;
            u64 bound_ubo_offset;

            u16 attribute_stride;

            ShaderState state;
    };

}
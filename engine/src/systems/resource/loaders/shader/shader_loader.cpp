#include "shader_loader.hpp"

#include "core/logger/logger.hpp"
#include "platform/filesystem.hpp"
#include "core/utils/string.hpp"
#include "systems/resource/resources/shader/shader_resource.hpp"

namespace Engine {

    ShaderLoader::ShaderLoader(u32 id, std::string type_path) : ResourceLoader(id, ResourceType::MATERIAL, type_path) {

    };

    ShaderLoader::ShaderLoader(u32 id, std::string type_path, std::string custom_type) : ResourceLoader(id, ResourceType::MATERIAL, type_path, custom_type) {

    };

    Resource* ShaderLoader::Load(std::string name) {
        std::string file_path = StringFormat("%s/%s.shdc", type_path.c_str(), name.c_str());
        WARN("%s", file_path.c_str());
        tinyxml2::XMLDocument* file = FileSystem::OpenXml(file_path);

        // TODO: Use of versions
        // tinyxml2::XMLElement* version = file->FirstChildElement("Version");
        tinyxml2::XMLElement* shader = file->FirstChildElement("Shader");
        if (!shader) {
            ERROR("ShaderLoader::Load - error opening file '%s'.", name.c_str());
            return nullptr;
        }

        ShaderConfig data = {};
        std::string buffer;
        data.name = shader->Attribute("name");
        data.renderpass_name = shader->Attribute("renderpass");
        buffer = shader->Attribute("use_instances");
        Parse(buffer, &data.use_instances);
        buffer = shader->Attribute("use_local");
        Parse(buffer, &data.use_local);

        // Shader stages
        tinyxml2::XMLElement* shader_stages = shader->FirstChildElement("Stages");
        if (shader_stages) {
            tinyxml2::XMLElement* shader_stage = shader_stages->FirstChildElement("Stage");
            while (shader_stage) {
                ShaderStageConfig shader_stage_config = {};
                shader_stage_config.file_path = shader_stage->Attribute("file");
                shader_stage_config.name = shader_stage->Attribute("type");
                std::string stage_type = shader_stage->Attribute("type");
                if (stage_type == "fragment" || stage_type == "frag") {
                    shader_stage_config.stage = ShaderStage::FRAGMENT;
                } else if (stage_type == "vertex" || stage_type == "vert") {
                    shader_stage_config.stage = ShaderStage::VERTEX;
                } else if (stage_type == "geometry" || stage_type == "geom") {
                    shader_stage_config.stage = ShaderStage::GEOMETRY;
                } else if (stage_type == "compute" || stage_type == "comp") {
                    shader_stage_config.stage = ShaderStage::COMPUTE;
                }

                if (!(u32)shader_stage_config.stage) {
                    ERROR("ShaderLoader::Load - '%s' - unknown shader stage type.", stage_type.c_str());
                } else {
                    data.stages.push_back(shader_stage_config);
                }
                shader_stage = shader_stage->NextSiblingElement();
            }   
        }

        // Shader attributes
        tinyxml2::XMLElement* shader_attributes = shader->FirstChildElement("Attributes");
        if (shader_attributes) {
            tinyxml2::XMLElement* shader_attribute = shader_attributes->FirstChildElement("Attribute");
            while (shader_attribute) {
                ShaderAttrConfig attr_config = {};
                attr_config.name = shader_attribute->Attribute("name");
                std::string attr_type = shader_attribute->Attribute("type");

                if (attr_type == "f32") {
                    attr_config.type = ShaderAttributeType::FLOAT32;
                    attr_config.size = sizeof(f32);
                } else if (attr_type == "vec2") {
                    attr_config.type = ShaderAttributeType::FLOAT32_2;
                    attr_config.size = sizeof(f32) * 2;
                } else if (attr_type == "vec3") {
                    attr_config.type = ShaderAttributeType::FLOAT32_3;
                    attr_config.size = sizeof(f32) * 3;
                } else if (attr_type == "vec4") {
                    attr_config.type = ShaderAttributeType::FLOAT32_4;
                    attr_config.size = sizeof(f32) * 4;
                } else if (attr_type == "u8") {
                    attr_config.type = ShaderAttributeType::UINT8;
                    attr_config.size = sizeof(u8);
                } else if (attr_type == "u16") {
                    attr_config.type = ShaderAttributeType::UINT16;
                    attr_config.size = sizeof(u16);
                } else if (attr_type == "u32") {
                    attr_config.type = ShaderAttributeType::UINT32;
                    attr_config.size = sizeof(u16);
                } else if (attr_type == "i8") {
                    attr_config.type = ShaderAttributeType::INT8;
                    attr_config.size = sizeof(i8);
                } else if (attr_type == "i16") {
                    attr_config.type = ShaderAttributeType::INT16;
                    attr_config.size = sizeof(i16);
                } else if (attr_type == "i32") {
                    attr_config.type = ShaderAttributeType::INT32;
                    attr_config.size = sizeof(i32);
                } else {
                    ERROR("ShaderLoader::Load - '%s' unknown attribute type. Valid types: f32, vec2, vec3, vec4, u8, u16, u32, i8, i16, i32", attr_type.c_str());
                    WARN("ShaderLoader::Load - using f32 instead.");
                    attr_config.type = ShaderAttributeType::FLOAT32;
                    attr_config.size = sizeof(f32);
                }
                
                data.attributes.push_back(attr_config);

                shader_attribute = shader_attribute->NextSiblingElement();
            }
        }

        // Shader uniforms
        tinyxml2::XMLElement* shader_uniforms = shader->FirstChildElement("Uniforms");
        if (shader_uniforms) {
            tinyxml2::XMLElement* shader_uniform = shader_uniforms->FirstChildElement("Uniform");
            while (shader_uniform) {
                ShaderUniformConfig uniform_config = {};
                uniform_config.name = shader_uniform->Attribute("name");
                std::string uniform_type = shader_uniform->Attribute("type");

                std::string scope = shader_uniform->Attribute("scope");
                if (scope == "global") {
                    uniform_config.scope = ShaderScope::GLOBAL;
                } else if (scope == "instance") {
                    uniform_config.scope = ShaderScope::INSTANCE;
                } else if (scope == "local") {
                    uniform_config.scope = ShaderScope::LOCAL;
                } else {
                    ERROR("ShaderLoader::Load - '%s' unknown scope.", scope.c_str());
                    WARN("ShaderLoader::Load - using 'global' instead.");
                    uniform_config.scope = ShaderScope::GLOBAL;
                }

                if (uniform_type == "f32") {
                    uniform_config.type = ShaderUniformType::FLOAT32;
                    uniform_config.size = sizeof(f32);
                } else if (uniform_type == "vec2") {
                    uniform_config.type = ShaderUniformType::FLOAT32_2;
                    uniform_config.size = sizeof(f32) * 2;
                } else if (uniform_type == "vec3") {
                    uniform_config.type = ShaderUniformType::FLOAT32_3;
                    uniform_config.size = sizeof(f32) * 3;
                } else if (uniform_type == "vec4") {
                    uniform_config.type = ShaderUniformType::FLOAT32_4;
                    uniform_config.size = sizeof(f32) * 4;
                } else if (uniform_type == "u8") {
                    uniform_config.type = ShaderUniformType::UINT8;
                    uniform_config.size = sizeof(u8);
                } else if (uniform_type == "u16") {
                    uniform_config.type = ShaderUniformType::UINT16;
                    uniform_config.size = sizeof(u16);
                } else if (uniform_type == "u32") {
                    uniform_config.type = ShaderUniformType::UINT32;
                    uniform_config.size = sizeof(u16);
                } else if (uniform_type == "i8") {
                    uniform_config.type = ShaderUniformType::INT8;
                    uniform_config.size = sizeof(i8);
                } else if (uniform_type == "i16") {
                    uniform_config.type = ShaderUniformType::INT16;
                    uniform_config.size = sizeof(i16);
                } else if (uniform_type == "i32") {
                    uniform_config.type = ShaderUniformType::INT32;
                    uniform_config.size = sizeof(i32);
                } else if (uniform_type == "mat4") {
                    uniform_config.type = ShaderUniformType::MATRIX_4x4;
                    uniform_config.size = sizeof(glm::mat4);
                } else if (uniform_type == "samp" || uniform_type == "sampler") {
                    uniform_config.type = ShaderUniformType::SAMPLER;
                    uniform_config.size = 0;

                    if (uniform_config.scope == ShaderScope::INSTANCE && !data.use_instances) {
                        ERROR("ShaderLoader::Load - unable to load instance sampler. use_instances == false for shader '%s'", data.name.c_str());
                        shader_uniform = shader_uniform->NextSiblingElement();
                        continue;
                    }

                    if (uniform_config.scope == ShaderScope::LOCAL) {
                        ERROR("ShaderLoader::Load - sampler can't use local scope.");
                        shader_uniform = shader_uniform->NextSiblingElement();
                        continue;
                    }

                } else {
                    ERROR("ShaderLoader::Load - '%s' unknown uniform type. Valid types: f32, vec2, vec3, vec4, u8, u16, u32, i8, i16, i32, mat4, sampler", uniform_type.c_str());
                    WARN("ShaderLoader::Load - using f32 instead.");
                    uniform_config.type = ShaderUniformType::FLOAT32;
                    uniform_config.size = sizeof(f32);
                }

                data.uniforms.push_back(uniform_config);

                shader_uniform = shader_uniform->NextSiblingElement();
            }

        }

        FileSystem::CloseXml(file);

        return new ShaderResource(id, name, file_path, data);
    };

};
#pragma once

#include "defines.hpp"

#include "resources/texture/texture.hpp"
#include "core/utils/freelist.hpp"
#include "resources/shader/shader.hpp"

namespace Engine {

    #define DEFAULT_MATERIAL_NAME "default_material"

    class TextureMap {
        public:
            Texture* texture;
            TextureUse use;
    };

    struct MaterialCreateInfo {
        std::string name;
        std::vector<TextureMap> textures;
        Shader* shader;
        glm::vec4 diffuse_color;
        f32 shininess;
    };

    class Material {
        public:
            Material(MaterialCreateInfo& info);
            virtual ~Material();

            std::string& GetName() { return name; };
            Shader* GetShader() { return shader; };
            u32 GetId() { return id; };
            u32 GetInternalId() { return interanal_id; };
            u32 GetGeneration() { return generation; };
            glm::vec4 GetDiffuseColor() { return diffuse_color; };
            TextureMap& GetDiffuseMap() { return diffuse_map; };
            TextureMap& GetSpecularMap() { return specular_map; };

            void SetFrame(u32 frame) { current_frame = frame; };
            u32 GetFrame() { return current_frame; }; 

            b8 AcquireInstanceResources();
            b8 ApplyInstance();
            b8 ApplyLocal(const glm::mat4* model);

            void SetInternalId(u32 id) { interanal_id = id; };
            void SetGeneration(u32 gen) { generation = gen; };
            void SetId(u32 id) { id = id; };
            void SetMemory(FreelistNode* memory) { this->memory = memory; };

            FreelistNode* GetMemory() { return memory; };

        protected:
            std::string name;
            Shader* shader;
            u32 id;
            u32 generation;
            u32 interanal_id;
            glm::vec4 diffuse_color;
            TextureMap diffuse_map;
            TextureMap specular_map;
            TextureMap normals_map;
            FreelistNode* memory;
            u32 current_frame;
            f32 shininess;
    };
};
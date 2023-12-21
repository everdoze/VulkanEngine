#pragma once

#include "defines.hpp"

#include "resources/texture/texture.hpp"

namespace Engine {

    #define DEFAULT_MATERIAL_NAME "default_material"

    enum class MaterialType { 
        WORLD,
        UI
    };

    struct MaterialConfig {
        std::string name;
        MaterialType type;
        b8 auto_release;
        glm::vec4 diffuse_color;
        std::string diffuse_map_name;
    };

    struct MaterialCreateInfo {
        std::string name;
        MaterialType type;
        TextureUse use;
        Texture* texture;
        glm::vec4 diffuse_color;
    };

    class TextureMap {
        public:
            Texture* texture;
            TextureUse use;
    };

    class Material {
        public:
            Material(MaterialCreateInfo& info);
            virtual ~Material();

            std::string& GetName() { return name; };
            MaterialType GetType() { return type; };
            u32 GetId() { return id; };
            u32 GetInternalId() { return interanal_id; };
            u32 GetGeneration() { return generation; };
            glm::vec4 GetDiffuseColor() { return diffuse_color; };
            TextureMap& GetDiffuseMap() { return diffuse_map; };

            void SetInternalId(u32 id) { interanal_id = id; };
            void SetGeneration(u32 gen) { generation = gen; };
            void SetId(u32 id) { id = id; };

        protected:
            std::string name;
            MaterialType type;
            u32 id;
            u32 generation;
            u32 interanal_id;
            glm::vec4 diffuse_color;
            TextureMap diffuse_map;
    };
};
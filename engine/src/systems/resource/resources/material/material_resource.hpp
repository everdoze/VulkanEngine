#pragma once

#include "defines.hpp"
#include "systems/resource/resources/base/resource.hpp"
#include "resources/material/material.hpp"

namespace Engine {

    struct ENGINE_API MaterialConfig {
        std::string name;
        std::string shader_name;
        b8 auto_release;
        glm::vec4 diffuse_color;
        std::string diffuse_map_name;
        std::string specular_map_name;
        std::string normal_map_name;
        f32 shininess;
    };

    class ENGINE_API MaterialResource : public Resource {
        public:
            MaterialResource(
                u32 loader_id, std::string name,
                std::string full_path, MaterialConfig config
            );
            ~MaterialResource();
            
            std::string GetMaterialName() { return data.name; };
            b8 IsAutoRelease() { return data.auto_release; };
            glm::vec4 GetDiffuseColor() { return data.diffuse_color; };
            std::string GetDiffuseMapName() { return data.diffuse_map_name; };

            MaterialConfig GetConfig() { 
                return data;
            };

        protected:
            MaterialConfig data;
    };

}
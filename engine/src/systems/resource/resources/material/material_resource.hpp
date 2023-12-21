#pragma once

#include "defines.hpp"
#include "systems/resource/resources/base/resource.hpp"
#include "resources/material/material.hpp"

namespace Engine {

    class MaterialResource : public Resource {
        public:
            MaterialResource(
                u32 loader_id, std::string name, MaterialType type,
                std::string full_path, std::string material_name, b8 auto_release,
                glm::vec4 diffuse_color, std::string diffuse_map_name
            );
            ~MaterialResource();
            
            std::string GetMaterialName() { return material_name; };
            b8 IsAutoRelease() { return auto_release; };
            glm::vec4 GetDiffuseColor() { return diffuse_color; };
            std::string GetDiffuseMapName() { return diffuse_map_name; };

            MaterialConfig GetConfig() { 
                return {
                    material_name,
                    type,
                    auto_release,
                    diffuse_color,
                    diffuse_map_name
                };
            };

        protected:
            std::string material_name;
            MaterialType type;
            b8 auto_release;
            glm::vec4 diffuse_color;
            std::string diffuse_map_name;
    };

}
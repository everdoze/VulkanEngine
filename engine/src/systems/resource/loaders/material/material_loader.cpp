#include "material_loader.hpp"

#include "core/logger/logger.hpp"
#include "platform/filesystem.hpp"

#include "systems/resource/resources/material/material_resource.hpp"

namespace Engine {
    MaterialLoader::MaterialLoader(u32 id, std::string type_path, std::string custom_type) : ResourceLoader(id, ResourceType::MATERIAL, type_path, custom_type) {};
    MaterialLoader::MaterialLoader(u32 id, std::string type_path) : ResourceLoader(id, ResourceType::MATERIAL, type_path) {};

    Resource* MaterialLoader::Load(std::string name) {
        std::string file_path = StringFormat("%s/%s.%s", type_path.c_str(), name.c_str(), "mat");
        File* file = FileSystem::FileOpen(
            file_path, 
            FileMode::READ, 
            false
        );

        if (!file->IsReady()) {
            ERROR("Unable to open material file '%s'", file_path.c_str());
            return nullptr;
        }

        std::string material_name;
        MaterialType type = MaterialType::WORLD;
        b8 auto_release = false;
        glm::vec4 diffuse_color;
        std::string diffuse_map_name;

        std::string line;
        while (file->ReadLine(line)) {
            line = LTrim(line);
            
            if (line[0] == '#' || (!line.length())) {
                continue;
            }

            auto pair = MidString(line, '=');

            if (pair.first == "version") {
                // TODO: handle versions
            } else if (pair.first == "name") {
                material_name = pair.second;
            } else if (pair.first == "diffuse_color") {
                if (!Parse(pair.second, &diffuse_color)) {
                    ERROR("Error occured when parsing parameter '%s' in material file '%s'.", pair.first.c_str(), file_path.c_str());
                }
            } else if (pair.first == "diffuse_map_name") {
                diffuse_map_name = pair.second;
            } else if (pair.first == "type") {
                if (pair.second == "ui") {
                    type = MaterialType::UI;
                } else if (pair.second == "world") {
                    type = MaterialType::WORLD;
                }
            }
        };

        FileSystem::FileClose(file);

        return new MaterialResource(id, name, type, file_path, material_name, auto_release, diffuse_color, diffuse_map_name);
    };

}

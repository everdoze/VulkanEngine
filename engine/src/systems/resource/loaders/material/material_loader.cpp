#include "material_loader.hpp"

#include "core/logger/logger.hpp"
#include "platform/filesystem.hpp"

#include "systems/resource/resources/material/material_resource.hpp"

namespace Engine {
    MaterialLoader::MaterialLoader(u32 id, std::string type_path, std::string custom_type) : ResourceLoader(id, ResourceType::MATERIAL, type_path, custom_type) {};
    MaterialLoader::MaterialLoader(u32 id, std::string type_path) : ResourceLoader(id, ResourceType::MATERIAL, type_path) {};

    Resource* MaterialLoader::Load(std::string name) {
        std::string file_path = StringFormat("%s/%s.%s", type_path.c_str(), name.c_str(), "mat");
        tinyxml2::XMLDocument* file = FileSystem::OpenXml(file_path);

        // TODO: Use of versions
        // tinyxml2::XMLElement* version = file->FirstChildElement("Version");
        tinyxml2::XMLElement* material = file->FirstChildElement("Material");
        if (!material) {
            ERROR("MaterialLoader::Load - error opening file '%s'.", name.c_str());
            return nullptr;
        }

        MaterialConfig data;
        std::string diffuse_color = material->Attribute("diffuse_color");
        if (!Parse(diffuse_color, &data.diffuse_color)) {
            ERROR("Error occured when parsing parameter 'diffuse_color' in material file '%s'.", diffuse_color.c_str(), file_path.c_str());
        }

        const char* shininess = material->Attribute("shininess");
        if (shininess) {
            std::string buf = shininess;
            if (!Parse(buf, &data.shininess)) {
                ERROR("Error occured when parsing parameter 'shininess' in material file '%s'.", diffuse_color.c_str(), file_path.c_str());
            }
        }

        const char* diffuse_name = material->Attribute("diffuse_map_name");
        if (diffuse_name) {
            data.diffuse_map_name = diffuse_name;
        }

        const char* normal_name = material->Attribute("normal_map_name");
        if (normal_name) {
            data.normal_map_name = normal_name;
        }

        const char* specular_name = material->Attribute("specular_map_name");
        if (specular_name) {
            data.specular_map_name = specular_name;
        }
        
        data.name = material->Attribute("name");
        
        tinyxml2::XMLElement* shader = material->FirstChildElement("Shader");
        if (shader) {
            data.shader_name = shader->Attribute("name");
        }

        FileSystem::CloseXml(file);

        return new MaterialResource(id, name, file_path, data);
    };

}

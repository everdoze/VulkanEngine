#pragma once

#include "defines.hpp"

#include "resources/material/material.hpp"
#include "systems/resource/resources/material/material_resource.hpp"

namespace Engine {

    struct MaterialReference {
        Material* material;
        b8 auto_release;
        u32 ref_count;
    };

    class ENGINE_API MaterialSystem {
        public:
            MaterialSystem();
            ~MaterialSystem();

            static b8 Initialize();
            static void Shutdown();
            static MaterialSystem* GetInstance() { return instance; };

            Material* AcquireMaterial(std::string name, b8 auto_release = true);
            void ReleaseMaterial(std::string name);

            Material* LoadMaterial(MaterialConfig& config);
            Material* GetDefaultMaterial() {return default_material; };


            void CreateDefaultMaterial();
            void DestroyDefaultMaterial();

            Material* AcquireMaterialFromConfig(MaterialConfig& config, b8 auto_release = true);

        private:
            static MaterialSystem* instance;

            Material* default_material;
            std::unordered_map<std::string, MaterialReference> registered_materials;
    };

};
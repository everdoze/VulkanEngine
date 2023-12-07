#pragma once

#include "defines.hpp"

#include "resources/material/material.hpp"

namespace Engine {

    class MaterialSystem {
        public:
            MaterialSystem();
            ~MaterialSystem();

            static b8 Initialize();
            static void Shutdown();
            static MaterialSystem* GetInstance() { return instance; };

            Material* AcquireMaterial(std::string name);
            void ReleaseMaterial(std::string name);

            Material* LoadMaterial(MaterialConfig& config);

            void CreateDefaultMaterial();
            void DestroyDefaultMaterial();

            Material* AcquireMaterialFromConfig(MaterialConfig& config);
            b8 LoadMaterialConfig(std::string file_path, MaterialConfig* out_config);

        private:
            static MaterialSystem* instance;

            Material* default_material;
            std::unordered_map<std::string, Material*> registered_materials;
    };

};
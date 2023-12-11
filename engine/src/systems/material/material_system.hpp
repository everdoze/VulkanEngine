#pragma once

#include "defines.hpp"

#include "resources/material/material.hpp"

namespace Engine {
    // TODO: AUTO_RELEASE usage
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
            Material* GetDefaultMaterial() {return default_material; };

            void CreateDefaultMaterial();
            void DestroyDefaultMaterial();

            Material* AcquireMaterialFromConfig(MaterialConfig& config);

        private:
            static MaterialSystem* instance;

            Material* default_material;
            std::unordered_map<std::string, Material*> registered_materials;
    };

};
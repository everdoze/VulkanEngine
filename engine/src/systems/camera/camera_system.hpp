#pragma once

#include "defines.hpp"
#include "camera/camera.hpp"

#define DEFAULT_CAMERA_NAME "default_camera"

namespace Engine {

    class ENGINE_API CameraSystem {
        public:
            CameraSystem();
            ~CameraSystem();

            static b8 Initialize();
            static void Shutdown();
            static CameraSystem* GetInstance() { return instance; };

            Camera* CreateCamera(std::string name, b8 register_camera = true);
            Camera* GetCamera (std::string name);
            void DisposeCamera (std::string name);
            Camera* GetDefaultCamera() { return &default_camera; };

            void SetActive(Camera* camera) { active = camera; };
            Camera* GetActive() { return active; };

        private:
            static CameraSystem* instance;
            std::unordered_map<std::string, Camera*> cameras;
            Camera default_camera;
            Camera* active;
    };

}
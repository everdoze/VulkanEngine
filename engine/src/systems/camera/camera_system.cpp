#include "camera_system.hpp"
#include "core/logger/logger.hpp"

namespace Engine {
    CameraSystem* CameraSystem::instance = nullptr;

    CameraSystem::CameraSystem() {
        default_camera = Camera();
        active = &default_camera;
    };

    CameraSystem::~CameraSystem() {
        for (const auto& [name, camera] : cameras) {
            DEBUG("|_Destroying camera '%s'.", name.c_str())
            delete camera;
        }
    };

    b8 CameraSystem::Initialize() {
        if (!instance) {
            instance = new CameraSystem();
            return true;
        }
        WARN("CameraSystem is already initialized.");
        return true;
    };


    void CameraSystem::Shutdown() {
        if (instance) {
            DEBUG("Shutting down CameraSystem.")
            return delete instance;
        }
        ERROR("CameraSystem is not initialized.");
    };


    Camera* CameraSystem::GetCamera (std::string name) {
        if (cameras.contains(name)) {
            return cameras[name];
        }
        ERROR("CameraSystem::GetCamera camera '%s' not found.", name.c_str())
        return nullptr;
    }

    void CameraSystem::DisposeCamera (std::string name) {
        if (cameras.contains(name)) {
            return delete cameras[name];
        }
        ERROR("CameraSystem::DisposeCamera camera '%s' does not exists.", name.c_str())
    };

    Camera* CameraSystem::CreateCamera(std::string name, b8 register_camera) {
        Camera* camera = new Camera();
        if (register_camera) {
            cameras[name] = camera;
        }
        return camera;
    };
}
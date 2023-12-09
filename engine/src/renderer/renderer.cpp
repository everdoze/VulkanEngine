#include "renderer.hpp"

#include "backend/vulkan/vulkan.hpp"
#include "core/logger/logger.hpp"
#include "platform/platform.hpp"
#include "core/utils/string.hpp"

// TODO: TEMP
#include "systems/material/material_system.hpp"
#include "core/event/event.hpp"
// TODO: TEMP END

namespace Engine {

    RendererFrontend* RendererFrontend::instance = nullptr;

    RendererFrontend* RendererFrontend::GetInstance() {
        if (instance) {
            return instance;
        }
        return nullptr;
    };

    b8 RendererFrontend::Initialize(RendererSetup setup, RendererBackendType type) {
        instance = new RendererFrontend();
        instance->CreateBackend(setup, type);
        instance->CreateCamera();
        instance->InitializeRenderer();
        
        return true;
    };

    void RendererFrontend::CreateCamera() {
        camera = new Camera();
    };

    void RendererFrontend::DestroyCamera() {
        delete camera;
    };
    
    void RendererFrontend::InitializeRenderer() {
        
    };

    b8 RendererFrontend::CreateBackend(RendererSetup setup, RendererBackendType type) {
        switch (type) {
            case RendererBackendType::VULKAN: {
                backend = new VulkanRendererBackend(setup);
            } break;

            case RendererBackendType::OPEN_GL: {
                backend = nullptr;
            } break;

            case RendererBackendType::DIRECT_X: {
                backend = nullptr;
            } break;
        }
        backend->Initialize();

        return true;
    };

    void RendererFrontend::Shutdown() {
        if (instance) {
            instance->ShutdownBackend();
            delete instance;
        }
    };

    void RendererFrontend::ShutdownBackend() {
        if (backend) {
            backend->Shutdown();
            delete backend;
        }
    };

    void RendererFrontend::Resized(u16 width, u16 height) {
        if (backend) {
            camera->OnResize();
            backend->Resized(width, height);
        }
    };
    
    b8 RendererFrontend::DrawFrame(RenderPacket* packet) {
        RendererFrontend* instance = RendererFrontend::GetInstance();

        if (instance->BeginFrame(packet->delta_time)) {
            
            instance->camera->OnUpdate();

            instance->backend->UpdateGlobalState(instance->camera->GetProjectionMatrix(), instance->camera->GetViewMatrix(), glm::vec3(), glm::vec4(1), 0);
            
            for (u32 i = 0; i < packet->geometries.size(); ++i) {
                instance->backend->DrawGeometry(packet->geometries[i]);
            }

            b8 result = instance->EndFrame(packet->delta_time);
            if (!result) {
                ERROR("RendererFrontend::EndFrame failed. Application shutting down...");
                return false;
            }
        }

        return true;
    };

    b8 RendererFrontend::BeginFrame(f32 delta_time) {
        if (!backend) {
            return false;
        }
        return backend->BeginFrame(delta_time);
    };

    b8 RendererFrontend::EndFrame(f32 delta_time) {
        if (!backend) {
            return false;
        }
        return backend->EndFrame(delta_time);
    };

    b8 RendererFrontend::UpdateGlobalState(glm::mat4 projection, glm::mat4 view, glm::vec3 view_position, glm::vec4 ambient_colour, i32 mode) {
        if (!backend) {
            return false;
        }
        return backend->UpdateGlobalState(projection, view, view_position, ambient_colour, mode);
    };

    void RendererFrontend::DrawGeometry(GeometryRenderData data) {
        if (!backend) {
            return;
        }
        return backend->DrawGeometry(data);
    };

    Texture* RendererFrontend::CreateTexture(TextureCreateInfo& info) {
        if (!backend) {
            return nullptr;
        }

        return backend->CreateTexture(info);
    };

    Material* RendererFrontend::CreateMaterial(MaterialCreateInfo& info) {
        if (!backend) {
            return nullptr;
        }

        return backend->CreateMaterial(info);
    };
    

    Geometry* RendererFrontend::CreateGeometry(GeometryCreateInfo& info) {
        if (!backend) {
            return nullptr;
        }
        return backend->CreateGeometry(info);
    };

};
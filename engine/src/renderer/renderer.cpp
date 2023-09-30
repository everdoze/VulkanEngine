#include "renderer.hpp"

#include "backend/vulkan/vulkan.hpp"
#include "core/logger/logger.hpp"
#include "platform/platform.hpp"
#include "core/utils/string.hpp"

// TODO: TEMP
#include "systems/texture/texture_system.hpp"
#include "core/event/event.hpp"
// TODO: TEMP END

namespace Engine {

    Ref<RendererFrontend> RendererFrontend::instance = nullptr;

    Ref<RendererFrontend> RendererFrontend::GetInstance() {
        if (instance) {
            return instance;
        }
        return nullptr;
    };

    // void RendererFrontend::SetViewMatrix(glm::mat4 view) {
    //     this->view = view;
    // }

    b8 RendererFrontend::OnDebugEvent(EventType type, EventContext context) {
        const char* names[2] = {
            "cobblestone",
            "paving"
        };

        Ref<TextureSystem> ts = TextureSystem::GetInstance();

        const char* old_name = names[current_texture];

        current_texture = (current_texture + 1) % 2;
        
        u32 old_gen = 0;
        if (test_texture) {
            old_gen = test_texture->GetGeneration();
        }

        test_texture = ts->AcquireTexture(names[current_texture], true);

        test_texture->SetGeneration(old_gen+1);
        
        ts->ReleaseTexture(old_name);

        return true;
    };

    b8 RendererFrontend::Initialize(RendererSetup setup, RendererBackendType type) {
        instance = CreateRef<RendererFrontend>();
        instance->CreateBackend(setup, type);
        instance->CreateCamera();
        instance->InitializeRenderer();
        
        return true;
    };

    void RendererFrontend::CreateCamera() {
        camera = CreateRef<Camera>();
    };

    void RendererFrontend::DestroyCamera() {
        camera = nullptr;
    };
    
    void RendererFrontend::InitializeRenderer() {
        EventSystem::GetInstance()->RegisterEvent(
            EventType::Debug1,
            "Renderer",
            EventBind(OnDebugEvent)
        );
    };

    b8 RendererFrontend::CreateBackend(RendererSetup setup, RendererBackendType type) {
        switch (type) {
            case RendererBackendType::VULKAN: {
                backend = CreateRef<VulkanRendererBackend>(setup);
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
            instance->test_texture = nullptr;
            EventSystem::GetInstance()->UnregisterEvent(
                EventType::Debug1,
                "Renderer"
            );
            instance->ShutdownBackend();
            instance = nullptr;
        }
    };

    void RendererFrontend::ShutdownBackend() {
        if (backend) {
            backend->Shutdown();
            backend = nullptr;
        }
    };

    void RendererFrontend::Resized(u16 width, u16 height) {
        if (backend) {
            camera->OnResize();
            backend->Resized(width, height);
        }
    };
    
    b8 RendererFrontend::DrawFrame(f32 delta_time) {
        Ref<RendererFrontend> instance = RendererFrontend::GetInstance();

        if (instance->BeginFrame(delta_time)) {
            
            instance->camera->OnUpdate();

            instance->backend->UpdateGlobalState(instance->camera->GetProjectionMatrix(), instance->camera->GetViewMatrix(), glm::vec3(), glm::vec4(1), 0);
            
            GeometryRenderData data = {};
            data.model = glm::mat4(1.0f);
            data.object_id = 0;
            data.textures[0] = instance->test_texture;
            instance->backend->UpdateObject(data);

            b8 result = instance->EndFrame(delta_time);
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

    void RendererFrontend::UpdateObject(GeometryRenderData data) {
        if (!backend) {
            return;
        }
        return backend->UpdateObject(data);
    };

    Ref<Texture> RendererFrontend::CreateTexture(
        std::string name,
        u32 width,
        u32 height,
        u8 channel_count,
        u8 has_transparency,
        u8* pixels) {

        if (!backend) {
            return nullptr;
        }

        return Cast<Texture>(backend->CreateTexture(
            name,
            width,
            height,
            channel_count,
            has_transparency,
            pixels
        ));
    };

};
#include "renderer.hpp"

#include "backend/vulkan/vulkan.hpp"
#include "core/logger/logger.hpp"
#include "platform/platform.hpp"
#include "core/utils/string.hpp"
#include "resources/mesh/mesh.hpp"

#include "systems/shader/shader_system.hpp"
// TODO: TEMP
#include "systems/material/material_system.hpp"
#include "core/event/event.hpp"
// TODO: TEMP END

namespace Engine {

    RendererFrontend* RendererFrontend::instance = nullptr;

    RendererFrontend::RendererFrontend() {
        shader_debug_mode = 0;

        EventSystem::GetInstance()->RegisterEvent(
            EventType::Debug2,
            "Renderer",
            EventBind(OnDebugEvent)
        );
    };

    RendererFrontend::~RendererFrontend() {
        EventSystem::GetInstance()->UnregisterEvent(
            EventType::Debug2,
            "Renderer"
        );
    };

    b8 RendererFrontend::OnDebugEvent(EventType type, EventContext& context) {
        shader_debug_mode++;
        shader_debug_mode %= 5;
        return true;
    };

    RendererFrontend* RendererFrontend::GetInstance() {
        if (instance) {
            return instance;
        }
        return nullptr;
    };
    

    b8 RendererFrontend::Initialize(RendererSetup setup, RendererBackendType type) {
        instance = new RendererFrontend();
        if (!instance->CreateBackend(setup, type)) {
            return false;
        };
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
        ShaderSystem* shader_system = ShaderSystem::GetInstance();
        Shader* material_shader = shader_system->CreateShader(BUILTIN_MATERIAL_SHADER_NAME);
        Shader* ui_shader = shader_system->CreateShader(BUILTIN_UI_SHADER_NAME);

        ambient_color = {0.25f, 0.25f, 0.25f, 1.0f};
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

        if (!backend->Initialize()) {
            return false;
        }

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
        ShaderSystem* shader_system = ShaderSystem::GetInstance();
        instance->backend->NextFrame();
        if (instance->BeginFrame(packet->delta_time)) {
            
            instance->camera->OnUpdate();

            // World renderpass begin
            if (!instance->backend->BeginRenderpass(BuiltinRenderpasses::WORLD)) {
                ERROR("backend->BeginRenderpass: BuiltinRenderpasses::WORLD failed. Application shutting down...");
                return false;
            }

            shader_system->UseShader(BUILTIN_MATERIAL_SHADER_NAME);
            
            ParamsData data(5);
            data[0] = (Param){"projection", &instance->camera->projection};
            data[1] = (Param){"view", &instance->camera->view};
            data[2] = (Param){"ambient_color", &instance->ambient_color};
            data[3] = (Param){"view_position", &instance->camera->camera_position};
            data[4] = (Param){"mode", &instance->shader_debug_mode};

            shader_system->ApplyGlobals(BUILTIN_MATERIAL_SHADER_NAME, data);
            u32 backend_frame = instance->backend->GetFrame();
            for (u32 i = 0; i < packet->geometries.size(); ++i) {
                if (!packet->geometries[i].geometry) {
                    ERROR("RendererFrontend::DrawFrame - something wrong with geometry at packet->geometries[%i], skipping...", i);
                    continue;
                }
                
                Material* material = packet->geometries[i].geometry->GetMaterial();
                if (!material) {
                    material = MaterialSystem::GetInstance()->GetDefaultMaterial();
                }

                if (material->GetFrame() != backend_frame) {
                    if (!material->ApplyInstance()) {
                        ERROR("Failed to apply material %s, skipping..", material->GetName().c_str());
                        continue;
                    }
                    material->SetFrame(backend_frame);
                }
                

                material->ApplyLocal(&packet->geometries[i].model);

                instance->backend->DrawGeometry(packet->geometries[i]);
            }
            
            if (!instance->backend->EndRenderpass(BuiltinRenderpasses::WORLD)) {
                ERROR("backend->EndRenderpass: BuiltinRenderpasses::WORLD failed. Application shutting down...");
                return false;
            }
            // World renderpass end

            // UI renderpass begin
            if (!instance->backend->BeginRenderpass(BuiltinRenderpasses::UI)) {
                ERROR("backend->BeginRenderpass: BuiltinRenderpasses::UI failed. Application shutting down...");
                return false;
            }

            shader_system->UseShader(BUILTIN_UI_SHADER_NAME);
            
            ParamsData ui_data(2);
            ui_data[0] = (Param){"projection", &instance->camera->ui_projection};
            ui_data[1] = (Param){"view", &instance->camera->ui_view};

            shader_system->ApplyGlobals(BUILTIN_UI_SHADER_NAME, ui_data);

            for (u32 i = 0; i < packet->ui_geometries.size(); ++i) {
                if (!packet->ui_geometries[i].geometry) {
                    ERROR("RendererFrontend::DrawFrame - something wrong with geometry at packet->ui_geometries[%i], skipping...", i);
                    continue;
                }

                Material* material = packet->ui_geometries[i].geometry->GetMaterial();
                if (!material) {
                    material = MaterialSystem::GetInstance()->GetDefaultMaterial();
                }

                if (material->GetFrame() != backend_frame) {
                    if (!material->ApplyInstance()) {
                        ERROR("Failed to apply material %s, skipping..", material->GetName().c_str());
                        continue;
                    }
                    material->SetFrame(backend_frame);
                }

                material->ApplyLocal(&packet->ui_geometries[i].model);

                instance->backend->DrawGeometry(packet->ui_geometries[i]);
            }

            if (!instance->backend->EndRenderpass(BuiltinRenderpasses::UI)) {
                ERROR("backend->EndRenderpass: BuiltinRenderpasses::UI failed. Application shutting down...");
                return false;
            }
            // UI renderpass end

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

    // b8 RendererFrontend::UpdateGlobalState(glm::mat4 projection, glm::mat4 view, glm::vec3 view_position, glm::vec4 ambient_colour, i32 mode) {
    //     if (!backend) {
    //         return false;
    //     }
    //     return backend->UpdateGlobalWorldState(projection, view, view_position, ambient_colour, mode);
    // };

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


    Shader* RendererFrontend::CreateShader(ShaderConfig& config)  {
        if (!backend) {
            return nullptr;
        }
        return backend->CreateShader(config);
    };

    u32 RendererFrontend::GetFrameHeightS() {
        if (!instance) {
            ERROR("RendererFrontend::GetFrameHeight - frontend not initialized!");
            return 0;
        }
        return instance->GetFrameHeight();
    };

    u32 RendererFrontend::GetFrameWidthS() {
        if (!instance) {
            ERROR("RendererFrontend::GetFrameHeight - frontend not initialized!");
            return 0;
        }
        return instance->GetFrameWidth();
    };

};
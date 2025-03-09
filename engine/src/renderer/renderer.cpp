#include "renderer.hpp"

#include "backend/vulkan/vulkan.hpp"
#include "core/logger/logger.hpp"
#include "platform/platform.hpp"
#include "core/utils/string.hpp"
#include "resources/mesh/mesh.hpp"

#include "systems/shader/shader_system.hpp"
#include "systems/camera/camera_system.hpp"
// TODO: TEMP
#include "systems/material/material_system.hpp"
#include "core/event/event.hpp"
// TODO: TEMP END

namespace Engine {

    RendererFrontend* RendererFrontend::instance = nullptr;

    RendererFrontend::RendererFrontend(RendererSetup* setup) {
        shader_debug_mode = 0;

        framebuffer_height = setup->height;
        framebuffer_width = setup->width;
        frames_since_resizing = 0;
        resizing = false;
        camera_system = nullptr;

        CreateEventListeners();
    };

    void RendererFrontend::CreateEventListeners() {
         EventSystem::GetInstance()->RegisterEvent(
            EventType::Debug2,
            "Renderer",
            EventBind(OnDebugEvent)
        );

        EventSystem::GetInstance()->RegisterEvent(
            EventType::RenderTargetsRefresh,
            "Renderer",
            EventBind(OnRegenerateRenderTargets)
        );
    };

    RendererFrontend::~RendererFrontend() {
        EventSystem::GetInstance()->UnregisterEvent(
            EventType::Debug2,
            "Renderer"
        );

        EventSystem::GetInstance()->UnregisterEvent(
            EventType::RenderTargetsRefresh,
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
        instance = new RendererFrontend(&setup);
        if (!instance->CreateBackend(setup, type)) {
            return false;
        };
        instance->GetRenderpasses();
        instance->InitializeRenderer();
        
        return true;
    };

    void RendererFrontend::GetRenderpasses() {
        world_renderpass = backend->GetRenderpass(world_renderpass_name);
        ui_renderpass = backend->GetRenderpass(ui_renderpass_name);

        RegenerateRenderTargets();
 
        // Update the main/world renderpass dimensions.
        world_renderpass->render_area.x = 0;
        world_renderpass->render_area.y = 0;
        world_renderpass->render_area.z = framebuffer_width;
        world_renderpass->render_area.w = framebuffer_height;
    
        // Also update the UI renderpass dimensions.
        ui_renderpass->render_area.x = 0;
        ui_renderpass->render_area.y = 0;
        ui_renderpass->render_area.z = framebuffer_width;
        ui_renderpass->render_area.w = framebuffer_height;
    };

    void RendererFrontend::GetCameraSystem() {
        camera_system = CameraSystem::GetInstance();
    };
    
    void RendererFrontend::InitializeRenderer() {
        ShaderSystem* shader_system = ShaderSystem::GetInstance();
        Shader* material_shader = shader_system->CreateShader(BUILTIN_MATERIAL_SHADER_NAME);
        Shader* ui_shader = shader_system->CreateShader(BUILTIN_UI_SHADER_NAME);

        ambient_color = {0.25f, 0.25f, 0.25f, 1.0f};
    };

    RendererInitializationSetup RendererFrontend::CreateInitSetup() {
        RendererInitializationSetup setup;

        std::vector<Engine::RenderpassCreateInfo> renderpass_config;
        renderpass_config.resize(2);

        renderpass_config[0].name = world_renderpass_name;
        renderpass_config[0].clear_color = glm::vec4(0.0f, 0.0f, 0.2f, 1.0f);
        renderpass_config[0].render_area = glm::vec4(0, 0, framebuffer_width, framebuffer_height);
        renderpass_config[0].next_name = ui_renderpass_name;
        renderpass_config[0].clear_flags = 
            Engine::RenderpassClearFlag::CLEAR_COLOR_BUFFER | 
            Engine::RenderpassClearFlag::CLEAR_COLOR_DEPTH_BUFER | 
            Engine::RenderpassClearFlag::CLEAR_COLOR_STENCIL_BUFFER;

        renderpass_config[1].name = ui_renderpass_name;
        renderpass_config[1].clear_color = glm::vec4(0.0f, 0.0f, 0.2f, 1.0f);
        renderpass_config[1].render_area = glm::vec4(0, 0, framebuffer_width, framebuffer_height);
        renderpass_config[1].prev_name = world_renderpass_name;
        renderpass_config[1].clear_flags = Engine::RenderpassClearFlag::CLEAR_NONE;

        setup.renderpasses = renderpass_config;

        return setup;
    };

    b8 RendererFrontend::CreateBackend(RendererSetup setup, RendererBackendType type) {
        switch (type) {
            case RendererBackendType::VULKAN: {
                backend = VulkanRendererBackend::CreateBackend(setup);
            } break;

            case RendererBackendType::OPEN_GL: {
                backend = nullptr;
            } break;

            case RendererBackendType::DIRECT_X: {
                backend = nullptr;
            } break;
        }

        RendererInitializationSetup init_setup = CreateInitSetup();
        if (!backend->Initialize(init_setup)) {
            return false;
        }

        image_count = backend->GetImageCount();

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
        framebuffer_height = height;
        framebuffer_width = width;
        frames_since_resizing = 0;
        resizing = true;
    };
    
    b8 RendererFrontend::DrawFrame(RenderPacket* packet) {
        return instance->_DrawFrame(packet);
    };

    b8 RendererFrontend::_DrawFrame(RenderPacket* packet) {
        if (!camera_system) {
            GetCameraSystem();
        }

        if (resizing) {
            frames_since_resizing++;
 
            // If the required number of frames have passed since the resize, go ahead and perform the actual updates.
            if (frames_since_resizing >= 30) {
               f32 width = framebuffer_width;
               f32 height = framebuffer_height;
               camera_system->GetActive()->OnResize(width, height);
               backend->Resized(width, height);
               world_renderpass->OnResize(glm::vec4(0, 0, width, height));
               ui_renderpass->OnResize(glm::vec4(0, 0, width, height));
    
               frames_since_resizing = 0;
               resizing = false;
            } else {
             // Skip rendering the frame and try again next time.
                return true;
            }
        }

        ShaderSystem* shader_system = ShaderSystem::GetInstance();
        backend->NextFrame();
        if (BeginFrame(packet->delta_time)) {
            
            camera_system->GetActive()->OnUpdate();

            // World renderpass begin
            if (!world_renderpass->Begin()) {
                ERROR("world_renderpass->Begin: BuiltinRenderpasses::WORLD failed. Application shutting down...");
                return false;
            }

            shader_system->UseShader(BUILTIN_MATERIAL_SHADER_NAME);
            
            ParamsData data(5);
            data[0] = (Param){"projection", &camera_system->GetActive()->projection};
            data[1] = (Param){"view", &camera_system->GetActive()->view};
            data[2] = (Param){"ambient_color", &ambient_color};
            data[3] = (Param){"view_position", &camera_system->GetActive()->camera_position};
            data[4] = (Param){"mode", &shader_debug_mode};

            shader_system->ApplyGlobals(BUILTIN_MATERIAL_SHADER_NAME, data);
            u32 backend_frame = backend->GetFrame();
            for (u32 i = 0; i < meshes.size(); ++i) {
                if (!meshes[i]) {
                    ERROR("RendererFrontend::DrawFrame - something wrong with a mesh at meshes[%i], skipping...", i);
                    continue;
                }
                Mesh* mesh = meshes[i];
                for (u32 j = 0; j < mesh->geometries.size(); ++j) {
                    Material* material = mesh->geometries[j]->GetMaterial();
                    if (!material) {
                        material = MaterialSystem::GetInstance()->GetDefaultMaterial();
                    }

                    material->ApplyInstance(backend_frame);

                    glm::mat4 model = mesh->transform->GetWorld();
                    material->ApplyLocal(&model);

                    backend->DrawGeometry({j, mesh->transform->GetWorld(), mesh->geometries[j]});
                }

            }
            
            if (!world_renderpass->End()) {
                ERROR("world_renderpass->End: BuiltinRenderpasses::WORLD failed. Application shutting down...");
                return false;
            }
            // World renderpass end

            // UI renderpass begin
            if (!ui_renderpass->Begin()) {
                ERROR("ui_renderpass->Begin: BuiltinRenderpasses::UI failed. Application shutting down...");
                return false;
            }

            shader_system->UseShader(BUILTIN_UI_SHADER_NAME);
            
            ParamsData ui_data(2);
            ui_data[0] = (Param){"projection", &camera_system->GetActive()->ui_projection};
            ui_data[1] = (Param){"view", &camera_system->GetActive()->ui_view};

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

                material->ApplyInstance(backend_frame);

                material->ApplyLocal(&packet->ui_geometries[i].model);

                backend->DrawGeometry(packet->ui_geometries[i]);
            }

            if (!ui_renderpass->End()) {
                ERROR("ui_renderpass->End: BuiltinRenderpasses::UI failed. Application shutting down...");
                return false;
            }
            // UI renderpass end

            b8 result = EndFrame(packet->delta_time);
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

    Sampler* RendererFrontend::CreateSampler(SamplerCreateInfo info) {
        if (!backend) {
            return nullptr;
        }
        return backend->CreateSampler(info);
    };

    RenderTarget* RendererFrontend::CreateRenderTarget(RenderTargetCreateInfo& info) {
        if (!backend) {
            return nullptr;
        }
        return backend->CreateRenderTarget(info);
    };

    Renderpass* RendererFrontend::CreateRenderpass(RenderpassCreateInfo& info) {
        if (!backend) {
            return nullptr;
        }
        return backend->CreateRenderpass(info);
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

    b8 RendererFrontend::OnRegenerateRenderTargets(EventType type, EventContext& context) {
        return RegenerateRenderTargets();
    };

    b8 RendererFrontend::RegenerateRenderTargets() {
        if (world_renderpass->render_targets.size() < image_count) {
            world_renderpass->render_targets.resize(image_count);
        }
        if (ui_renderpass->render_targets.size() < image_count) {
            ui_renderpass->render_targets.resize(image_count);
        }

        for (u32 i = 0; i < image_count; ++i) {
            delete world_renderpass->render_targets[i];
            delete ui_renderpass->render_targets[i];

            Texture* window_target_texture = backend->GetWindowAttachment(i);
            Texture* depth_target_texture = backend->GetDepthAttachment();

            RenderTargetCreateInfo world_create_info;
            world_create_info.attachments = {window_target_texture, depth_target_texture};
            world_create_info.height = framebuffer_height;
            world_create_info.width = framebuffer_width;
            world_create_info.manage_attachments = false;
            world_create_info.renderpass = world_renderpass;

            world_renderpass->render_targets[i] = backend->CreateRenderTarget(world_create_info);

            RenderTargetCreateInfo ui_create_info;
            ui_create_info.attachments = {window_target_texture};
            ui_create_info.height = framebuffer_height;
            ui_create_info.width = framebuffer_width;
            ui_create_info.manage_attachments = false;
            ui_create_info.renderpass = ui_renderpass;

            ui_renderpass->render_targets[i] = backend->CreateRenderTarget(ui_create_info);
        }

        return true;
    }
};
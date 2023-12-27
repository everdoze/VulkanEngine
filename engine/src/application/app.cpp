#include "app.hpp"

#include "core/logger/logger.hpp"
#include "platform/platform.hpp"
#include "renderer/renderer.hpp"

#include "systems/texture/texture_system.hpp"
#include "systems/material/material_system.hpp"
#include "systems/geometry/geometry_system.hpp"
#include "systems/resource/resource_system.hpp"
#include "systems/shader/shader_system.hpp"

#include "platform/filesystem.hpp"

namespace Engine {

    b8 Application::RegisterEvents() {
        EventSystem* event = EventSystem::GetInstance();

        // On application quit
        event->RegisterEvent(
            EventType::AppQuit, 
            "Application", 
            EventBind(onExit)
        );

        event->RegisterEvent(
            EventType::KeyPressed, 
            "Application", 
            EventBind(onKey)
        );

        event->RegisterEvent(
            EventType::KeyReleased, 
            "Application", 
            EventBind(onKey)
        );

        event->RegisterEvent(
            EventType::WindowResize, 
            "Application", 
            EventBind(onResize)
        );

        event->RegisterEvent(
            EventType::Debug1,
            "Application",
            EventBind(OnDebugEvent)
        );

        return true;
    };

    b8 Application::OnDebugEvent(EventType type, EventContext& context) {
        const int names_count = 2;
        const char* names[names_count] = {
            "cobblestone",
            "paving"
        };

        TextureSystem* ts = TextureSystem::GetInstance();
        Texture* texture = test_geometry->GetMaterial()->GetDiffuseMap().texture;

        const char* old_name = current_texture < names_count ? names[current_texture] : nullptr;

        current_texture = (current_texture + 1) % 2;
        
        u32 old_gen = 0;
        if (texture) {
            old_gen = texture->GetGeneration();
        }

        texture = ts->AcquireTexture(names[current_texture], true);

        texture->SetGeneration(old_gen+1);
        test_geometry->GetMaterial()->GetDiffuseMap().texture = texture;
        if (old_name) {
            ts->ReleaseTexture(old_name);
        }
        
        return true;
    };


    Camera* Application::GetCamera() {
        return RendererFrontend::GetInstance()->GetCamera();
    };

    Application::Application(ApplicationSetup setup, ApplicationCommandLineArgs args) {
        this->setup = setup;

        Logger::Initialize();

        try {
            if (!InputSystem::Initialize()) {
                SetReady(false);
                FATAL("Error during InputSystem initialization.");
                return;
            }
            
            if (!EventSystem::Initialize()) {
                SetReady(false);
                FATAL("Error during EventSystem initialization.");
                return;
            }

            // Events registration
            RegisterEvents();
            
            if (!Platform::Initialize(setup.name, setup.start_x, setup.start_y, setup.width, setup.height)) {
                SetReady(false);
                FATAL("Error during Platform initialization.");
                return;
            }

            if (!ResourceSystem::Initialize("../assets")) {
                SetReady(false);
                FATAL("Error during ResourceSystem initialization.");
                return;
            }

            if (!ShaderSystem::Initialize()) {
                SetReady(false);
                FATAL("Error during ShaderSystem initialization.");
                return;
            }

            RendererSetup renderer_setup;
            renderer_setup.height = setup.height;
            renderer_setup.width = setup.width;
            renderer_setup.name = setup.name;
            if (!RendererFrontend::Initialize(renderer_setup, RendererBackendType::VULKAN)) {
                SetReady(false);
                FATAL("Error during Renderer initialization.");
                return;
            }

            if (!TextureSystem::Initialize()) {
                SetReady(false);
                FATAL("Error during TextureSystem initialization.");
                return;
            }

            if (!MaterialSystem::Initialize()) {
                SetReady(false);
                FATAL("Error during MaterialSystem initialization.");
                return;
            }

            if (!GeometrySystem::Initialize()) {
                SetReady(false);
                FATAL("Error during GeometrySystem initialization.")
            }
            
            // TODO: temp
            GeometrySystem* gs = GeometrySystem::GetInstance();
            GeometryConfig g_config = gs->GeneratePlainConfig(10.0f, 10.0f, 5, 5, 20.0f, 20.0f, "test_geometry", "test");
            test_geometry = gs->AcquireGeometryFromConfig(g_config, true);

            Platform::FMemory(g_config.indices); 
            Platform::FMemory(g_config.vertices); 

            // Load up some test UI geometry.
            GeometryConfig ui_config;
            ui_config.vertex_size = sizeof(Vertex2D);
            ui_config.vertex_count = 4;
            ui_config.index_size = sizeof(u32);
            ui_config.index_count = 6;
            ui_config.material_name = "test_ui";
            ui_config.name = "test_ui_geometry";

            const f32 f = 256.0f;
            std::vector<Vertex2D> uiverts(4);
            uiverts[0].position.x = 0.0f;  // 0    3
            uiverts[0].position.y = 0.0f;  //
            uiverts[0].texcoord.x = 0.0f;  //
            uiverts[0].texcoord.y = 0.0f;  // 2    1

            uiverts[1].position.y = f;
            uiverts[1].position.x = f;
            uiverts[1].texcoord.x = 1.0f;
            uiverts[1].texcoord.y = 1.0f;

            uiverts[2].position.x = 0.0f;
            uiverts[2].position.y = f;
            uiverts[2].texcoord.x = 0.0f;
            uiverts[2].texcoord.y = 1.0f;

            uiverts[3].position.x = f;
            uiverts[3].position.y = 0.0;
            uiverts[3].texcoord.x = 1.0f;
            uiverts[3].texcoord.y = 0.0f;
            ui_config.vertices = uiverts.data();

            // Indices - counter-clockwise
            u32 uiindices[6] = {2, 1, 0, 3, 0, 1};
            ui_config.indices = uiindices;

            // Get UI geometry from config.
            test_ui_geometry = gs->AcquireGeometryFromConfig(ui_config, true);
            
            // TODO: temp

            SetReady(true);
            DEBUG("Application successfully initialized.");
        
        } catch (std::exception& ex) {
            ERROR("Application::Application - %s", ex.what());
            SetReady(false);
        } catch (...) {
            SetReady(false);
        }
    };

    void Application::SetReady(b8 value) {
        is_ready = value;
    };

    b8 Application::IsReady() {
        return is_ready;
    };

    void Application::Run () {
        this->running = true;
        this->clock.Start();
        this->clock.Update();
        this->last_time = this->clock.GetElapsed();

        f64 running_time = 0;
        u8 frame_count = 0;
        f64 target_frame_seconds = 1.0f / 170; // Max frame rate

        while (this->running) {
            if (!Platform::PumpMessages()) {
                this->running = false;
            }

            if (!this->suspended) {
                this->clock.Update();
                f64 current_time = this->clock.GetElapsed();
                f64 delta = (current_time - this->last_time);
                f64 frame_start_time = Platform::GetAbsoluteTime();

                GameUpdate(delta);

                RenderPacket packet;
                packet.delta_time = delta;

                GeometryRenderData test_render;
                test_render.geometry = test_geometry;
                test_render.model = glm::identity<glm::mat4>();
                
                packet.geometries.push_back(test_render);

                GeometryRenderData test_ui_render;
                test_ui_render.geometry = test_ui_geometry;
                test_ui_render.model = glm::identity<glm::mat4>();

                packet.ui_geometries.push_back(test_ui_render);

                RendererFrontend::DrawFrame(&packet);

                f64 frame_end_time = Platform::GetAbsoluteTime();
                f64 frame_elapsed_time = frame_end_time - frame_start_time;
                running_time += frame_elapsed_time;
                f64 remaining_time = target_frame_seconds - frame_elapsed_time;

                if (remaining_time > 0) {
                    u64 remaining_ms = (remaining_time * 1000);

                    b8 limit_frames = false;
                    if (remaining_ms > 0 && limit_frames) {
                        Platform::Sleep(remaining_ms - 1);
                    }   

                    frame_count++;
                }

                InputSystem::GetInstance()->InputUpdate(delta);

                last_time = current_time;
            }

        }

        this->Shutdown();
    };

    void Application::Stop () {
        if (this->running) {
            this->running = false;
        }
    };

    void Application::Shutdown () {
        // Unregister events /////////////
        EventSystem::GetInstance()->UnregisterEvent(EventType::AppQuit, "Application");
        EventSystem::GetInstance()->UnregisterEvent(EventType::KeyPressed, "Application");
        EventSystem::GetInstance()->UnregisterEvent(EventType::KeyReleased, "Application");
        EventSystem::GetInstance()->UnregisterEvent(EventType::WindowResize, "Application");
        EventSystem::GetInstance()->UnregisterEvent(EventType::Debug1, "Application");
        //////////////////////////////////

        GeometrySystem::Shutdown();
        TextureSystem::Shutdown();
        ShaderSystem::Shutdown();
        RendererFrontend::Shutdown();
        EventSystem::Shutdown();
        ResourceSystem::Shutdown();
        Platform::Shutdown();
        Logger::Shutdown();
    };

    b8 Application::onExit(EventType type, EventContext& context) {
        Stop();
        return true;
    };

    b8 Application::onKey(EventType type, EventContext& context) {

        if (type == EventType::KeyPressed) {
            u16 key_code = context.data.u16[0];
            if (key_code == Keys::KEYBOARD_ESCAPE) {
                // NOTE: Technically firing an event to itself, but there may be other listeners.
                EventContext data = {};
                EventSystem::GetInstance()->FireEvent(EventType::AppQuit, data);

                // Block anything else from processing this.
                return true;
            } else if (key_code == Keys::KEYBOARD_A) {
                // Example on checking for a key
                DEBUG("Explicit - A key pressed!");
            } else {
                DEBUG("'%c' key pressed in window.", key_code);
            }
        } else if (type == EventType::KeyReleased) {
            u16 key_code = context.data.u16[0];
            if (key_code == Keys::KEYBOARD_B) {
                // Example on checking for a key
                DEBUG("Explicit - B key released!");
            } else {
                DEBUG("'%c' key released in window.", key_code);
            }
        }
        return false;
    };

    b8 Application::onResize(EventType type, EventContext& context) {
        u16 width = context.data.u16[0];
        u16 height = context.data.u16[1];
        if (width != setup.width || height != setup.height) {
            setup.width = width;
            setup.height = height;

            DEBUG("Window resize event captured: %i, %i", width, height);

            if (width == 0 || height == 0) {
                DEBUG("Window collapsed. Suspending...");
                this->SetSuspended(true);
                return true;
            } else {
                if (this->GetSuspended()) {
                    DEBUG("Window restored. Resuming...");
                    this->SetSuspended(false);
                }

                RendererFrontend::GetInstance()->Resized(width, height);

                return true;
            }
        }
        return false;
    };

};

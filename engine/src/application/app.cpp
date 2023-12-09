#include "app.hpp"

#include "core/logger/logger.hpp"
#include "platform/platform.hpp"
#include "renderer/renderer.hpp"

#include "systems/texture/texture_system.hpp"
#include "systems/material/material_system.hpp"
#include "systems/geometry/geometry_system.hpp"

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
        // const int names_count = 2;
        // const char* names[names_count] = {
        //     "cobblestone",
        //     "paving"
        // };

        // TextureSystem* ts = TextureSystem::GetInstance();

        // const char* old_name = current_texture < names_count ? names[current_texture] : nullptr;

        // current_texture = (current_texture + 1) % 2;
        
        // u32 old_gen = 0;
        // if (test_texture) {
        //     old_gen = test_texture->GetGeneration();
        // }

        // test_texture = ts->AcquireTexture(names[current_texture], true);

        // test_texture->SetGeneration(old_gen+1);

        // if (old_name) {
        //     ts->ReleaseTexture(old_name);
        // }
        
        // return true;
        return true;
    };


    Camera* Application::GetCamera() {
        return RendererFrontend::GetInstance()->GetCamera();
    };

    Application::Application(ApplicationSetup setup, ApplicationCommandLineArgs args) {
        this->setup = setup;

        Logger::Initialize();

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
        test_geometry = GeometrySystem::GetInstance()->GetDefaultGeometry();
        // TODO: temp

        SetReady(true);
        DEBUG("Application successfully initialized.");
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

        TextureSystem::Shutdown();
        RendererFrontend::Shutdown();
        EventSystem::Shutdown();
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

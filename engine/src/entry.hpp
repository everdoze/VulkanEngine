#include "defines.hpp"
#include "core/clock/clock.hpp"
#include "core/input/input.hpp"
#include "core/event/event.hpp"
#include "platform/platform.hpp"
#include "camera/camera.hpp"
#include "resources/mesh/mesh.hpp"
#include "systems/texture/texture_system.hpp"
#include "systems/material/material_system.hpp"
#include "systems/geometry/geometry_system.hpp"
#include "systems/resource/resource_system.hpp"
#include "systems/camera/camera_system.hpp"
#include "systems/shader/shader_system.hpp"
#include "renderer/renderer.hpp"
#include "renderer/renderpass.hpp"


// extern Engine::Application* Engine::CreateApplication(Engine::ApplicationCommandLineArgs args);

const char APP[12] = "Application";

struct ApplicationSetup {
    std::string name;
    u32 width;
    u32 height;
    u32 start_x;
    u32 start_y;
};

struct ApplicationCommandLineArgs
{
	int Count = 0;
	char** Args = nullptr;

	const char* operator[](int index) const
	{
		return Args[index];
	}
};

template<typename T>
concept GameConcept = requires(T game, float delta, Engine::InputSystem* input_system) {
    { game.Initialize() } -> std::convertible_to<b8>;
    { game.Update(delta, input_system) } -> std::convertible_to<b8>;
	{ game.Shutdown() } -> std::convertible_to<void>;
};

template<GameConcept T>
class EngineRunner {
public:
    EngineRunner(const ApplicationSetup& setup);
    void Run(ApplicationCommandLineArgs args);

private:
	T game;

    ApplicationSetup m_setup;
	b8 running = false;
	b8 suspended = false;
	f64 last_time;

	Engine::Clock clock;

	b8 RegisterEvents();
	void Shutdown();
	b8 InitializeEngineSystems();

	b8 onExit(Engine::EventType type, Engine::EventContext& context);
    b8 onKey(Engine::EventType type, Engine::EventContext& context);
    b8 onResize(Engine::EventType type, Engine::EventContext& context);
};

template<GameConcept T>
EngineRunner<T>::EngineRunner(const ApplicationSetup& setup) 
    : m_setup(setup) {};

template<GameConcept T>
void EngineRunner<T>::Run(ApplicationCommandLineArgs args) {
	if (!InitializeEngineSystems()) {
		return FATAL("Engine systems initialization falied.");
	};

    if (!game.Initialize()) {
		return FATAL("Game initialization falied, should return 'true'.");
	};

	Engine::InputSystem* input_system = Engine::InputSystem::GetInstance();

	running = true;
	clock.Start();
    clock.Update();

	f64 running_time = 0;
    u8 frame_count = 0;
    f64 target_frame_seconds = 1.0f / 140; // Max frame rate
    b8 limit_frames = false; // limit or not

    while (running) {
        if (!Engine::Platform::PumpMessages()) {
            running = false;
			break;
        }

		if (!suspended) {
			clock.Update();

			f64 current_time = clock.GetElapsed();
            f64 delta = (current_time - last_time);
            f64 frame_start_time = Engine::Platform::GetAbsoluteTime();

			if (!game.Update(delta, input_system)) {
				running = false;
				break;
			} 
			
			Engine::RenderPacket packet;
            packet.delta_time = delta;
			Engine::RendererFrontend::DrawFrame(&packet);

			f64 frame_end_time = Engine::Platform::GetAbsoluteTime();
            f64 frame_elapsed_time = frame_end_time - frame_start_time;
            running_time += frame_elapsed_time;
            f64 remaining_time = target_frame_seconds - frame_elapsed_time;

            if (remaining_time > 0) {
                u64 remaining_ms = (remaining_time * 1000);

                if (remaining_ms > 0 && limit_frames) {
                    Engine::Platform::PSleep(remaining_ms - 1);
                }   

                frame_count++;
            }

            Engine::InputSystem::GetInstance()->InputUpdate(delta);

            last_time = current_time;
		}
    }

	Shutdown();
}

template<GameConcept T>
b8 EngineRunner<T>::RegisterEvents() {
    Engine::EventSystem* event = Engine::EventSystem::GetInstance();

    // On application quit
    event->RegisterEvent(
        Engine::EventType::AppQuit, 
        APP, 
        EventBind(onExit)
    );

    event->RegisterEvent(
        Engine::EventType::KeyPressed, 
        APP, 
        EventBind(onKey)
    );

    event->RegisterEvent(
        Engine::EventType::KeyReleased, 
        APP, 
        EventBind(onKey)
    );

    event->RegisterEvent(
        Engine::EventType::WindowResize, 
        APP, 
        EventBind(onResize)
    );

    return true;
};

template<GameConcept T>
void EngineRunner<T>::Shutdown () {
	game.Shutdown();

    // Unregister events /////////////
    Engine::EventSystem::GetInstance()->UnregisterEvent(Engine::EventType::AppQuit, "Application");
    Engine::EventSystem::GetInstance()->UnregisterEvent(Engine::EventType::KeyPressed, "Application");
    Engine::EventSystem::GetInstance()->UnregisterEvent(Engine::EventType::KeyReleased, "Application");
    Engine::EventSystem::GetInstance()->UnregisterEvent(Engine::EventType::WindowResize, "Application");
    //////////////////////////////////

    Engine::CameraSystem::Shutdown();
    Engine::GeometrySystem::Shutdown();
    Engine::TextureSystem::Shutdown();
    Engine::ShaderSystem::Shutdown();
    Engine::RendererFrontend::Shutdown();
    Engine::EventSystem::Shutdown();
    Engine::ResourceSystem::Shutdown();
    Engine::Platform::Shutdown();
    Engine::Logger::Shutdown();
}

template<GameConcept T>
bool EngineRunner<T>::InitializeEngineSystems() {
    Engine::Logger::Initialize();

	if (!Engine::InputSystem::Initialize()) {
		FATAL("Error during InputSystem initialization.");
		return false;
	}

	if (!Engine::EventSystem::Initialize()) {
		FATAL("Error during EventSystem initialization.");
        return false;
	}

	if (!RegisterEvents()) {	
		FATAL("Error during registering events.");
        return false;
	}

	if (!Engine::Platform::Initialize(
		m_setup.name, m_setup.start_x, 
		m_setup.start_y, m_setup.width, 
		m_setup.height)) {
		FATAL("Error during Platform initialization.");
        return false;
	}

	if (!Engine::ResourceSystem::Initialize("../assets")) {
		FATAL("Error during ResourceSystem initialization.");
        return false;
	}

	if (!Engine::ShaderSystem::Initialize()) {
		FATAL("Error during ShaderSystem initialization.");
        return false;
	}

	if (!Engine::RendererFrontend::Initialize({m_setup.width, m_setup.height, m_setup.name}, Engine::RendererBackendType::VULKAN)) {
		FATAL("Error during Renderer initialization.");
        return false;
	}

	if (!Engine::TextureSystem::Initialize()) {
		FATAL("Error during TextureSystem initialization.");
    	return false;
	}

	if (!Engine::MaterialSystem::Initialize()) {
		FATAL("Error during MaterialSystem initialization.");
        return false;
	}

	if (!Engine::GeometrySystem::Initialize()) {
        FATAL("Error during GeometrySystem initialization.");
		return false;
    }

    if (!Engine::CameraSystem::Initialize()) {
        FATAL("Error during CameraSystem initialization.");
        return false;
    }

	DEBUG("Application successfully initialized.");

	return true;
}

template<GameConcept T>
b8 EngineRunner<T>::onExit(Engine::EventType type, Engine::EventContext& context) {
    running = false;
    return true;
};

template<GameConcept T>
b8 EngineRunner<T>::onKey(Engine::EventType type, Engine::EventContext& context) {
    if (type == Engine::EventType::KeyPressed) {
        u16 key_code = context.data.u16[0];
        if (key_code == Engine::Keys::KEYBOARD_ESCAPE) {
            // NOTE: Technically firing an event to itself, but there may be other listeners.
            Engine::EventContext data = {};
            Engine::EventSystem::GetInstance()->FireEvent(Engine::EventType::AppQuit, data);

            // Block anything else from processing this.
            return true;
        } else if (key_code == Engine::Keys::KEYBOARD_A) {
            // Example on checking for a key
            DEBUG("Explicit - A key pressed!");
        } else {
            DEBUG("'%c' key pressed in window.", key_code);
        }
    } else if (type == Engine::EventType::KeyReleased) {
        u16 key_code = context.data.u16[0];
        if (key_code == Engine::Keys::KEYBOARD_B) {
            // Example on checking for a key
            DEBUG("Explicit - B key released!");
        } else {
            DEBUG("'%c' key released in window.", key_code);
        }
    }
    return false;
};

template<GameConcept T>
b8 EngineRunner<T>::onResize(Engine::EventType type, Engine::EventContext& context) {
    u16 width = context.data.u16[0];
    u16 height = context.data.u16[1];
    if (width != m_setup.width || height != m_setup.height) {
        m_setup.width = width;
        m_setup.height = height;

        DEBUG("Window resize event captured: %i, %i", width, height);

        if (width == 0 || height == 0) {
            DEBUG("Window collapsed. Suspending...");
            suspended = true;
            return true;
        } else {
            if (suspended) {
                DEBUG("Window restored. Resuming...");
                suspended = false;
            }

            Engine::RendererFrontend::GetInstance()->Resized(width, height);

            return true;
        }
    }
    return false;
};
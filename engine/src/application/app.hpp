#pragma once

#include "defines.hpp"
#include "core/clock/clock.hpp"
#include "core/input/input.hpp"
#include "core/event/event.hpp"
#include "camera/camera.hpp"

int main(int argc, char** argv);

namespace Engine {

    struct ApplicationSetup {
        std::string name;
        i32 width;
        i32 height;
        i32 start_x;
        i32 start_y;
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

    class ENGINE_API Application {
        public:
            static void CreateApplication();

            Application(ApplicationSetup setup = {"Unnamed", 800, 600, 100, 100}, ApplicationCommandLineArgs args = ApplicationCommandLineArgs());
            virtual ~Application() = default;

            ApplicationSetup& GetSetup() { return setup; };

            virtual b8 GameUpdate(f32 delta) = 0;
            virtual b8 GameInitialize() = 0;

            void SetSuspended (b8 suspended) { this->suspended = suspended; };
            b8 GetSuspended () { return suspended; };
            
            Ref<Camera> GetCamera();

            static Ref<Application> GetInstance();

            void SetReady(b8 value);
            b8 IsReady();
            void Run();
            void Shutdown();
            void Stop();

        protected:
            ApplicationSetup setup;

            b8 RegisterEvents();

            b8 onExit(EventType type, EventContext& context);
            b8 onKey(EventType type, EventContext& context);
            b8 onResize(EventType type, EventContext& context);

        private:
            b8 running = false;
            b8 suspended = false;
            b8 is_ready = false;

            Clock clock;

            
            f64 last_time;

            static Ref<Application> instance;
            friend int ::main(int argc, char** argv);
    };


    Ref<Application> CreateApplication(ApplicationCommandLineArgs args);
}
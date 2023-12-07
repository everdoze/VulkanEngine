#include "application/app.hpp"

extern Engine::Application* Engine::CreateApplication(Engine::ApplicationCommandLineArgs args);


int main(int argc, char** argv)
{
	auto app = Engine::CreateApplication({ argc, argv });
	app->GameInitialize();
	if (app->IsReady()) {
		app->Run();
	} else {
		app->Shutdown();
	}
	delete app;
}

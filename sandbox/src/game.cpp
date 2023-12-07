#include <entry.hpp>
#include <core/logger/logger.hpp>

class Test : public Engine::Application {
    using Engine::Application::Application;

    public:
        Engine::Camera* camera;

        b8 GameInitialize() {
            camera = GetCamera();
            return true;
        };

        b8 GameUpdate(f32 delta) {
            Engine::InputSystem* input = Engine::InputSystem::GetInstance();

            if (input->IsKeyUp(Engine::Keys::KEYBOARD_T) && input->WasKeyDown(Engine::Keys::KEYBOARD_T)) {
                Engine::EventSystem::GetInstance()->FireEvent(Engine::EventType::Debug1, {});
            }

            if (input->IsKeyDown(Engine::Keys::KEYBOARD_A) || input->IsKeyDown(Engine::Keys::KEYBOARD_LEFT)) {
                camera->Yaw(-1.0f * delta);
            }

            if (input->IsKeyDown(Engine::Keys::KEYBOARD_D) || input->IsKeyDown(Engine::Keys::KEYBOARD_RIGHT)) {
                camera->Yaw(1.0f * delta);
            }

            if (input->IsKeyDown(Engine::Keys::KEYBOARD_UP)) {
                camera->Pitch(-1.0f * delta);
            }

            if (input->IsKeyDown(Engine::Keys::KEYBOARD_DOWN)) {
                camera->Pitch(1.0f * delta);
            }

            f32 temp_move_speed = 50.0f;
            glm::vec3 velocity(0);

            if (input->IsKeyDown(Engine::Keys::KEYBOARD_W)) {
                camera->MoveForward(temp_move_speed * delta);
            }

            if (input->IsKeyDown(Engine::Keys::KEYBOARD_S)) {
                camera->MoveBackward(temp_move_speed * delta);
            }

            if (input->IsKeyDown(Engine::Keys::KEYBOARD_Q)) {
                camera->MoveLeft(temp_move_speed * delta);
            }

            if (input->IsKeyDown(Engine::Keys::KEYBOARD_E)) {
                camera->MoveRight(temp_move_speed * delta);
            }

            if (input->IsKeyDown(Engine::Keys::KEYBOARD_SPACE)) {
                camera->MoveUp(temp_move_speed * delta);
            }

            if (input->IsKeyDown(Engine::Keys::KEYBOARD_X)) {
                camera->MoveDown(temp_move_speed * delta);
            }

            return true;
        };


};

Engine::Application* Engine::CreateApplication(Engine::ApplicationCommandLineArgs args) {
    Engine::ApplicationSetup setup;
    setup.height = 600;
    setup.width = 800;
    setup.start_x = 100;
    setup.start_y = 100;
    setup.name = "name";
    return new Test(setup, args);
}

#include <entry.hpp>
#include <core/logger/logger.hpp>
#include <platform/platform.hpp>
#include <renderer/renderer.hpp>

class Test : public Engine::Application {
    using Engine::Application::Application;

    public:
        Engine::Camera* camera;

        b8 lock_cursor = true;

        b8 GameInitialize() {
            camera = GetCamera();
            Engine::Platform::CursorVisibility(false);
            return true;
        };

        b8 GameUpdate(f32 delta) {
            Engine::InputSystem* input = Engine::InputSystem::GetInstance();

            // Engine::Logger::GetLogger()->Info("Camera position: (x: %f, y: %f, z: %f)", 
            // camera->camera_position.x,
            // camera->camera_position.y, 
            // camera->camera_position.z);

            if (lock_cursor) {
                Engine::MousePosition prev_pos = input->GetPreviousMousePosition();
                Engine::MousePosition current_pos = input->GetMousePosition();
                i32 delta_x = current_pos.x - prev_pos.x;
                i32 delta_y = current_pos.y - prev_pos.y;

                if (!prev_pos.x  && !prev_pos.y) {
                    delta_x = 0;
                    delta_y = 0;
                }

                if (delta_x != 0) {
                    camera->Yaw(1 * delta_x * delta);
                }
                
                if (delta_y != 0) {
                    camera->Pitch(1 * delta_y * delta);
                }
            }
            
            
            if (input->IsKeyUp(Engine::Keys::KEYBOARD_L) && input->WasKeyDown(Engine::Keys::KEYBOARD_L)) {
                if (lock_cursor) {
                    lock_cursor = false;
                    Engine::Platform::CursorVisibility(true);
                } else {
                    lock_cursor = true;
                    Engine::Platform::CursorVisibility(false);
                }
            }

            if (input->IsKeyUp(Engine::Keys::KEYBOARD_T) && input->WasKeyDown(Engine::Keys::KEYBOARD_T)) {
                Engine::EventSystem::GetInstance()->FireEvent(Engine::EventType::Debug1, {});
            }

            if (input->IsKeyUp(Engine::Keys::KEYBOARD_R) && input->WasKeyDown(Engine::Keys::KEYBOARD_R)) {
                Engine::EventSystem::GetInstance()->FireEvent(Engine::EventType::Debug3, {});
            }

            if (input->IsKeyUp(Engine::Keys::KEYBOARD_M) && input->WasKeyDown(Engine::Keys::KEYBOARD_M)) {
                Engine::EventSystem::GetInstance()->FireEvent(Engine::EventType::Debug2, {});
            }

            if (input->IsKeyDown(Engine::Keys::KEYBOARD_LEFT)) {
                camera->Yaw(-1.0f * delta);
            }

            if (input->IsKeyDown(Engine::Keys::KEYBOARD_RIGHT)) {
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

            if (input->IsKeyDown(Engine::Keys::KEYBOARD_A)) {
                camera->MoveLeft(temp_move_speed * delta);
            }

            if (input->IsKeyDown(Engine::Keys::KEYBOARD_D)) {
                camera->MoveRight(temp_move_speed * delta);
            }

            if (input->IsKeyDown(Engine::Keys::KEYBOARD_SPACE)) {
                camera->MoveUp(temp_move_speed * delta);
            }

            if (input->IsKeyDown(Engine::Keys::KEYBOARD_X)) {
                camera->MoveDown(temp_move_speed * delta);
            }

            if (lock_cursor) {
                u32 frame_height = GetFrameHeight();
                u32 frame_width = GetFrameWidth();
                Engine::Platform::SetCursorPosition(frame_width * 0.5, frame_height * 0.5);
            }
            
            return true;
        };


};

Engine::Application* Engine::CreateApplication(Engine::ApplicationCommandLineArgs args) {
    Engine::ApplicationSetup setup;
    setup.height = 800;
    setup.width = 1280;
    setup.start_x = 100;
    setup.start_y = 100;
    setup.name = "name";
    return new Test(setup, args);
};

#include "game.hpp"
#include "systems/camera/camera_system.hpp"

bool Game::Initialize() {
    RegisterEvents();
    cs = Engine::CameraSystem::GetInstance();

    cs->CreateCamera("Camera1")->SetPosition(glm::vec3(5, 0, 0));
    cs->CreateCamera("Camera2")->SetPosition(glm::vec3(15, 0, 0));
    cs->CreateCamera("Camera3")->SetPosition(glm::vec3(10, 0, 0));

    Engine::Platform::CursorVisibility(true);
    demo = new DemoScene();
    return true;
}

bool Game::Update(float delta, Engine::InputSystem* input) {
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
            cs->GetActive()->Yaw(1 * delta_x * delta);
        }
                
        if (delta_y != 0) {
            cs->GetActive()->Pitch(1 * delta_y * delta);
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

    if (input->IsKeyUp(Engine::Keys::KEYBOARD_C) && input->WasKeyDown(Engine::Keys::KEYBOARD_C)) {
        Engine::EventSystem::GetInstance()->FireEvent(Engine::EventType::Debug5, {});
    }


    if (input->IsKeyDown(Engine::Keys::KEYBOARD_LEFT)) {
        cs->GetActive()->Yaw(-1.0f * delta);
    }

    if (input->IsKeyDown(Engine::Keys::KEYBOARD_RIGHT)) {
        cs->GetActive()->Yaw(1.0f * delta);
    }

    if (input->IsKeyDown(Engine::Keys::KEYBOARD_UP)) {
        cs->GetActive()->Pitch(-1.0f * delta);
    }

    if (input->IsKeyDown(Engine::Keys::KEYBOARD_DOWN)) {
        cs->GetActive()->Pitch(1.0f * delta);
    }

    f32 temp_move_speed = 50.0f;
    glm::vec3 velocity(0);

    if (input->IsKeyDown(Engine::Keys::KEYBOARD_W)) {
        cs->GetActive()->MoveForward(temp_move_speed * delta);
    }

    if (input->IsKeyDown(Engine::Keys::KEYBOARD_S)) {
        cs->GetActive()->MoveBackward(temp_move_speed * delta);
    }

    if (input->IsKeyDown(Engine::Keys::KEYBOARD_A)) {
        cs->GetActive()->MoveLeft(temp_move_speed * delta);
    }

    if (input->IsKeyDown(Engine::Keys::KEYBOARD_D)) {
        cs->GetActive()->MoveRight(temp_move_speed * delta);
    }

    if (input->IsKeyDown(Engine::Keys::KEYBOARD_SPACE)) {
        cs->GetActive()->MoveUp(temp_move_speed * delta);
    }

    if (input->IsKeyDown(Engine::Keys::KEYBOARD_X)) {
        cs->GetActive()->MoveDown(temp_move_speed * delta);
    }

    if (lock_cursor) {
        u32 frame_height = Engine::RendererFrontend::GetFrameWidthS();
        u32 frame_width = Engine::RendererFrontend::GetFrameWidthS();
        Engine::Platform::SetCursorPosition(frame_width * 0.5, frame_height * 0.5);
    }
            
    return true;
};

void Game::Shutdown () {
    Engine::EventSystem::GetInstance()->UnregisterEvent(Engine::EventType::Debug1, "Game");
    Engine::EventSystem::GetInstance()->UnregisterEvent(Engine::EventType::Debug3, "Game");
    Engine::EventSystem::GetInstance()->UnregisterEvent(Engine::EventType::Debug5, "Game");
    delete demo;
};

void Game::RegisterEvents () {
    Engine::EventSystem* event = Engine::EventSystem::GetInstance();

    event->RegisterEvent(
        Engine::EventType::Debug1,
        "Game",
        EventBind(OnDebugEvent)
    );

    event->RegisterEvent(
        Engine::EventType::Debug3,
        "Game",
        EventBind(OnDebugEvent2)
    );

    event->RegisterEvent(
        Engine::EventType::Debug5,
        "Game",
        EventBind(OnDebugEvent5)
    );
}

b8 Game::OnDebugEvent5(Engine::EventType type, Engine::EventContext& context) {
    curr = (curr + 1) % 4;
    std::vector<std::string> cameras = {"Camera1", "Camera2", "Camera3"};
    if (curr == cameras.size()) {
        cs->SetActive(cs->GetDefaultCamera());
    } else {
        cs->SetActive(cs->GetCamera(cameras[curr]));
    }

    return true;
};

b8 Game::OnDebugEvent2(Engine::EventType type, Engine::EventContext& context) {
    Engine::Logger::Debug("DebugEvent2");
    return true;
};

b8 Game::OnDebugEvent(Engine::EventType type, Engine::EventContext& context) {
    Engine::Logger::Debug("DebugEvent1");
    return true;
};

int main (int argc, char** argv) {
    ApplicationSetup setup{
        "Sandbox-App",
        1280, 800,
        100, 100
    };
    EngineRunner<Game> runner(setup);
    runner.Run({ argc, argv });
}
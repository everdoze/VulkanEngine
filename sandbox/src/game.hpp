#pragma once
#include <entry.hpp>
#include <core/logger/logger.hpp>
#include <platform/platform.hpp>
#include <renderer/renderer.hpp>
#include "demo_scene.hpp"

class Game {
public:
    b8 Initialize();
    b8 Update(float delta, Engine::InputSystem* input);
    void Shutdown();

private:
    void HandleCamera(float delta, Engine::InputSystem& input) {};
    void HandleInput(Engine::InputSystem& input) {};

    void RegisterEvents();

    b8 OnDebugEvent(Engine::EventType type, Engine::EventContext& context);
    b8 OnDebugEvent2(Engine::EventType type, Engine::EventContext& context);
    b8 OnDebugEvent5(Engine::EventType type, Engine::EventContext& context);
private:
    Engine::Camera* camera = nullptr;
    Engine::CameraSystem* cs = nullptr;
    b8 lock_cursor = false;
    DemoScene* demo;

    u32 curr = 3;
};
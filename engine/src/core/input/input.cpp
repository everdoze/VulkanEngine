#include "input.hpp"

#include "platform/platform.hpp"
#include "core/logger/logger.hpp"
#include "core/event/event.hpp"

namespace Engine {

    Ref<InputSystem> InputSystem::instance = nullptr;

    b8 InputSystem::Initialize() {
        if (!instance) {
            instance = CreateRef<InputSystem>();
        }
        DEBUG("Input system initialized successfully.");
        return true;
    };

    Ref<InputSystem> InputSystem::GetInstance() {
        if (instance) {
            return instance;
        }
        return nullptr;
    };

    void InputSystem::Shutdown() {
        instance = nullptr;
    };

    void InputSystem::InputUpdate(f64 delta_time) {
        Platform::CMemory(&this->keyboard_prev, &this->keyboard_current, sizeof(KeyboardState));
        Platform::CMemory(&this->mouse_prev, &this->mouse_current, sizeof(MouseState));
    };

    void InputSystem::ProcessKey(Keys key, b8 pressed) {
        if (key > Keys::KEYBOARD_MAX_KEYS) {
            return;
        }

        if (this->keyboard_current.keys[key] != pressed) {
            this->keyboard_current.keys[key] = pressed;

            EventContext context;
            context.data.u16[0] = key;
            EventSystem::GetInstance()->FireEvent(
                pressed ? EventType::KeyPressed : EventType::KeyReleased,
                context
            );
        }
    };

    void InputSystem::ProcessButton(Buttons button, b8 pressed) {
        if (this->mouse_current.buttons[button] != pressed) {
            this->mouse_current.buttons[button] = pressed;

            EventContext context;
            context.data.u16[0] = button;
            EventSystem::GetInstance()->FireEvent(
                pressed ? EventType::MouseButtonPressed : EventType::MouseButtonReleased,
                context
            );
        }
    };

    void InputSystem::ProcessMouseMove(i16 x, i16 y) {
        if (this->mouse_current.x != x || this->mouse_current.y != y) {
            // DEBUG("Mouse pos: %i, %i!", x, y);

            // Update internal input_state->
            this->mouse_current.x = x;
            this->mouse_current.y = y;

            // Fire the event.
            EventContext context;
            context.data.u16[0] = x;
            context.data.u16[1] = y;
            EventSystem::GetInstance()->FireEvent(EventType::MouseMoved, context);
        }
    };
    
    void InputSystem::ProcessMouseWheel(i8 z_delta) {
        EventContext context;
        context.data.u8[0] = z_delta;
        EventSystem::GetInstance()->FireEvent(EventType::MouseScrolled, context);
    };
    
    b8 InputSystem::IsKeyDown(Keys key) {
        return this->keyboard_current.keys[key] == true;
    };

    b8 InputSystem::IsKeyUp(Keys key) {
        return this->keyboard_current.keys[key] == false;
    };

    b8 InputSystem::WasKeyDown(Keys key) {
        return this->keyboard_prev.keys[key] == true;
    };

    b8 InputSystem::WasKeyUp(Keys key) {
        return this->keyboard_prev.keys[key] == false;
    };

    b8 InputSystem::IsButtonDown(Buttons button) {
        return this->mouse_current.buttons[button] == true;
    };

    b8 InputSystem::WasButtonDown(Buttons button) {
        return this->mouse_prev.buttons[button] == true;
    };

    MousePosition InputSystem::GetMousePosition() {
        return (MousePosition){this->mouse_current.x, this->mouse_current.y};
    };

    MousePosition InputSystem::GetPreviousMousePosition() {
        return (MousePosition){this->mouse_prev.x, this->mouse_prev.y};
    };

};
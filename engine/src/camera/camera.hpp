#pragma once

#include "defines.hpp"
#include "core/event/event.hpp"

namespace Engine {
    
    class ENGINE_API Camera {
        public:

            Camera();
            ~Camera();

            glm::quat GetOrientation();
            glm::vec3 GetUpDirection();
            glm::vec3 GetRightDirection();
            glm::vec3 GetForwardDirection();

            glm::mat4 GetProjectionMatrix() { return projection; };
            glm::mat4 GetViewMatrix() { return view; };

            glm::mat4 GetUIProjectionMatrix() { return ui_projection; };
            glm::mat4 GetUIViewMatrix() { return ui_view; };

            void MoveUp(f32 amount);
            void MoveDown(f32 amount);
            void MoveForward(f32 amount);
            void MoveBackward(f32 amount);
            void MoveLeft(f32 amount);
            void MoveRight(f32 amount);

            void Yaw(f32 amount);
            void Pitch(f32 amount);
            void Roll(f32 amount);

            void OnUpdate();
            void OnResize();

            b8 Test(EventType type, EventContext context);

            glm::mat4 projection;
            glm::mat4 view;

            glm::mat4 ui_projection;
            glm::mat4 ui_view;

            glm::vec3 camera_position;
            glm::vec3 camera_euler;
        private:
            void GenerateProjectionMatrix();
            void GenerateViewMatrix();

            void GenerateUIProjectionMatrix();
            void GenerateUIViewMatrix();

            // TODO: Probably need to configurate these
            f32 near_clip = 0.1f;
            f32 far_clip = 1000.0f;
            f32 fov = 45.0f;

            b8 camera_dirty = false;
    };

};

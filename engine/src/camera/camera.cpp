#include "camera.hpp"

#include "core/logger/logger.hpp"
#include "renderer/renderer.hpp"

namespace Engine {
    
    static const f32 limit = glm::radians(89.0f);

    Camera::Camera() {
        // EventSystem::GetInstance()->RegisterEvent(
        //     EventType::MouseMoved,
        //     "Camera",
        //     EventBind(Test)
        // );

        GenerateProjectionMatrix();
        GenerateViewMatrix();
    }

    Camera::~Camera() {
        // EventSystem::GetInstance()->UnregisterEvent(
        //     EventType::MouseMoved,
        //     "Camera"
        // );
    };
    
    void Camera::GenerateProjectionMatrix() {
        Ref<RendererFrontend> frontend = RendererFrontend::GetInstance();
        projection = glm::perspective(fov, frontend->GetFrameWidth() / (f32)frontend->GetFrameHeight(), near_clip, far_clip);
    };

    void Camera::GenerateViewMatrix() {
        glm::mat4 rotation = glm::toMat4(GetOrientation());

        glm::mat4 translation = glm::translate(glm::identity<glm::mat4>(), camera_position);

        view = translation * rotation;
        view = glm::inverse(view);
    };

    void Camera::OnResize() {
        GenerateProjectionMatrix();
    };

    glm::quat Camera::GetOrientation() {
        return glm::quat(-glm::vec3(camera_euler));
    };
    
    glm::vec3 Camera::GetUpDirection() {
        return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
    };

    glm::vec3 Camera::GetRightDirection() {
        return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
    };

    glm::vec3 Camera::GetForwardDirection() {
        return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
    };

    void Camera::OnUpdate() {
        if (camera_dirty) {
            GenerateViewMatrix();   
        }
    };

    b8 Camera::Test(EventType type, EventContext context) {
        u16 mouse_x = context.data.u16[0];
        u16 mouse_y = context.data.u16[1];

        INFO("MouseX: %u, MouseY: %u", mouse_x, mouse_y);

        return true;
    };

    void Camera::MoveUp(f32 amount) {
        camera_position += GetUpDirection() * amount;
        camera_dirty = true;
    };

    void Camera::MoveDown(f32 amount) {
        camera_position += -GetUpDirection() * amount;
        camera_dirty = true;
    };

    void Camera::MoveForward(f32 amount) {
        camera_position += GetForwardDirection() * amount;
        camera_dirty = true;
    };

    void Camera::MoveBackward(f32 amount) {
        camera_position += -GetForwardDirection() * amount;
        camera_dirty = true;
    };

    void Camera::MoveLeft(f32 amount) {
        camera_position += -GetRightDirection() * amount;
        camera_dirty = true;
    };

    void Camera::MoveRight(f32 amount) {
        camera_position += GetRightDirection() * amount;
        camera_dirty = true;
    };

    void Camera::Yaw(f32 amount) {
        camera_euler.y += amount;
        camera_dirty = true;
    };

    void Camera::Pitch(f32 amount) {
        camera_euler.x += amount;

        // Clamp to avoid Gimball lock
        camera_euler.x = Clamp(camera_euler.x, -limit, limit);
        
        camera_dirty = true;
    };

    void Camera::Roll(f32 amount) {
        camera_euler.z += amount;
        camera_dirty = true;
    };

};
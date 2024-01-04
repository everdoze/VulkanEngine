#include "transform.hpp"

namespace Engine {

    Transform::Transform() {
        position = glm::vec3(0, 0, 0);
        rotation = glm::identity<glm::quat>();
        scale = glm::vec3(1, 1, 1);
        model = glm::identity<glm::mat4>();
        parent = nullptr;
        Generate();
    };

    Transform::Transform(glm::vec3 position) {
        this->position = position;
        rotation = glm::identity<glm::quat>();
        scale = glm::vec3(1, 1, 1);
        model = glm::identity<glm::mat4>();
        parent = nullptr;
        Generate();
    };

    Transform::Transform(glm::vec3 position, glm::quat rotation) {
        this->position = position;
        this->rotation = rotation;
        scale = glm::vec3(1, 1, 1);
        model = glm::identity<glm::mat4>();
        parent = nullptr;
        Generate();
    };

    Transform::Transform(glm::vec3 position, glm::quat rotation, glm::vec3 scale) {
        this->position = position;
        this->rotation = rotation;
        this->scale = scale;
        model = glm::identity<glm::mat4>();
        parent = nullptr;
        Generate();
    };

    Transform::Transform(Transform* parent) {
        position = glm::vec3(0, 0, 0);
        rotation = glm::identity<glm::quat>();
        scale = glm::vec3(1, 1, 1);
        model = glm::identity<glm::mat4>();
        this->parent = parent;
        Generate();
    };

    Transform::Transform(glm::vec3 position, Transform* parent) {
        this->position = position;
        rotation = glm::identity<glm::quat>();
        scale = glm::vec3(1, 1, 1);
        model = glm::identity<glm::mat4>();
        this->parent = parent;
        Generate();
    };

    Transform::Transform(glm::vec3 position, glm::quat rotation, Transform* parent) {
        this->position = position;
        this->rotation = rotation;
        scale = glm::vec3(1, 1, 1);
        model = glm::identity<glm::mat4>();
        this->parent = parent;
        Generate();
    };

    Transform::Transform(glm::vec3 position, glm::quat rotation, glm::vec3 scale, Transform* parent) {
        this->position = position;
        this->rotation = rotation;
        this->scale = scale;
        model = glm::identity<glm::mat4>();
        this->parent = parent;
        Generate();
    };

    void Transform::Generate() {
        model = glm::toMat4(rotation) *  glm::translate(glm::identity<glm::mat4>(), position) * glm::scale(glm::identity<glm::mat4>(), scale);
        is_dirty = false;
    };

    void Transform::SetPosition(glm::vec3 position) {
        this->position = position;
        is_dirty = true;
    };

    void Transform::Translate(glm::vec3 position) {
        this->position += position;
        is_dirty = true;
    };

    void Transform::SetRotation(glm::quat rotation) {
        this->rotation = rotation;
        is_dirty = true;
    };

    void Transform::Rotate(glm::quat rotation) {
        this->rotation *= rotation;
        is_dirty = true;
    };

    void Transform::SetScale(glm::vec3 scale) {
        this->scale = scale;
        is_dirty = true;
    };

    void Transform::SetPositionRotation(glm::vec3 position, glm::quat rotation) {
        this->position = position;
        this->rotation = rotation;
        is_dirty = true;
    };  

    void Transform::SetPositionRotationScale(glm::vec3 position, glm::quat rotation, glm::vec3 scale) {
        this->position = position;
        this->rotation = rotation;
        is_dirty = true;
    };

    void Transform::SetParent(Transform* parent) {
        this->parent = parent;
    };

}
#pragma once

#include "defines.hpp"

namespace Engine {

    class Transform {
        public:
            Transform();
            Transform(glm::vec3 position);
            Transform(glm::vec3 position, glm::quat rotation);
            Transform(glm::vec3 position, glm::quat rotation, glm::vec3 scale);

            Transform(Transform* parent);
            Transform(glm::vec3 position, Transform* parent);
            Transform(glm::vec3 position, glm::quat rotation, Transform* parent);
            Transform(glm::vec3 position, glm::quat rotation, glm::vec3 scale, Transform* parent);

            virtual ~Transform() {};

            void SetPosition(glm::vec3 position);
            void Translate(glm::vec3 position);

            void SetRotation(glm::quat rotation);
            void Rotate(glm::quat rotation);

            void SetScale(glm::vec3 scale);
            void SetParent(Transform* parent);

            void SetPositionRotation(glm::vec3 position, glm::quat rotation);
            void SetPositionRotationScale(glm::vec3 position, glm::quat rotation, glm::vec3 scale);

            glm::vec3 GetPosition() { return position; };
            glm::quat GetRotation() { return rotation; };
            glm::vec3 GetScale() { return scale; };

            glm::mat4 GetLocal() {
                if (is_dirty) {
                    Generate();
                }
                return model;
            };

            glm::mat4 GetWorld() {
                glm::mat4 local = GetLocal();
                if (parent) {
                    return parent->GetWorld() * local;
                }
                return local;
            };

            Transform* GetParent() { return parent; };

        protected:
            void Generate();

            b8 is_dirty;
            glm::mat4 model;
            Transform* parent;
            glm::vec3 position;
            glm::quat rotation;
            glm::vec3 scale;

    };

}
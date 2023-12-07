#pragma once

#include "defines.hpp"
#include "camera/camera.hpp"
#include "resources/texture/texture.hpp"
#include "resources/material/material.hpp"
#include "renderer_types.inl"

namespace Engine {

    typedef enum RendererBackendType {
        VULKAN,
        OPEN_GL,
        DIRECT_X
    } RendererBackendType;

    typedef struct RendererSetup {
        u32 width;
        u32 height;
        std::string name;
    } RendererSetup;

    class RendererBackend {
        public:
            RendererBackend(RendererSetup setup) {
                name = setup.name;
                width = setup.width;
                height = setup.height;
            };

            virtual ~RendererBackend() = default;

            virtual b8 Initialize() = 0;
            virtual void Shutdown() = 0;
            virtual void Resized(u16 width, u16 height) = 0;
            virtual b8 BeginFrame(f32 delta_time) = 0;
            virtual b8 EndFrame(f32 delta_time) = 0;
            virtual b8 UpdateGlobalState(glm::mat4 projection, glm::mat4 view, glm::vec3 view_position, glm::vec4 ambient_colour, i32 mode) = 0;
            virtual void UpdateObject(GeometryRenderData data) = 0;

            u32 GetFrameWidth() { return width; };
            u32 GetFrameHeight() { return height; };

            virtual Texture* CreateTexture(TextureCreateInfo& info) = 0;
            virtual Material* CreateMaterial(MaterialCreateInfo& info) = 0;

        protected:
            std::string name;
            u32 width;
            u32 height;
            u32 cached_width;
            u32 cached_height;
    };

    class RendererFrontend {
        public: 
            static b8 Initialize(RendererSetup setup, RendererBackendType type);
            static void Shutdown();
            static RendererFrontend* GetInstance();
            static RendererBackend* GetBackend() { return instance->backend; };
            static b8 DrawFrame(f32 delta_time);

            void Resized(u16 width, u16 height);

            Camera* GetCamera() { return camera; };
            u32 GetFrameWidth() { return backend->GetFrameWidth(); };
            u32 GetFrameHeight() { return backend->GetFrameHeight(); };
            
            Texture* CreateTexture(TextureCreateInfo& info);
            Material* CreateMaterial(MaterialCreateInfo& info);

        private:
            b8 BeginFrame(f32 delta_time);
            b8 EndFrame(f32 delta_time);
            b8 UpdateGlobalState(glm::mat4 projection, glm::mat4 view, glm::vec3 view_position, glm::vec4 ambient_colour, i32 mode);
            void UpdateObject(GeometryRenderData data);

            void InitializeRenderer();

            b8 CreateBackend(RendererSetup setup, RendererBackendType type);
            void ShutdownBackend();

            b8 OnDebugEvent(EventType type, EventContext context);

            void CreateCamera();
            void DestroyCamera();

            static RendererFrontend* instance;
            RendererBackend* backend;

            Camera* camera;

            Texture* test_texture;

            u8 current_texture = 1;
    };

};
#pragma once

#include "defines.hpp"
#include "camera/camera.hpp"
#include "resources/texture/texture.hpp"
#include "resources/material/material.hpp"
#include "resources/geometry/geometry.hpp"
#include "resources/shader/shader.hpp"
#include "renderer_types.hpp"
#include "renderer/renderpass.hpp"
#include "systems/camera/camera_system.hpp"
// temp
#include "resources/mesh/mesh.hpp"

namespace Engine {

    enum RendererBackendType {
        VULKAN,
        OPEN_GL,
        DIRECT_X
    };

    struct RendererSetup {
        u32 width;
        u32 height;
        std::string name;
    };

    struct RendererInitializationSetup {
        std::vector<RenderpassCreateInfo> renderpasses;
    };

    class RendererBackend {
        public:
            RendererBackend(RendererSetup setup) {
                name = setup.name;
                width = setup.width;
                height = setup.height;
            };

            virtual ~RendererBackend() = default;

            virtual b8 Initialize(RendererInitializationSetup& setup) = 0;
            virtual void Shutdown() = 0;
            virtual void Resized(u16 width, u16 height) = 0;
            virtual b8 BeginFrame(f32 delta_time) = 0;
            virtual b8 EndFrame(f32 delta_time) = 0;
            virtual void DrawGeometry(GeometryRenderData data) = 0;
            virtual void NextFrame() = 0;
            virtual u32 GetFrame() = 0;
            virtual Renderpass* GetRenderpass(std::string name) = 0;
            virtual u32 GetImageCount() = 0;
            virtual Texture* GetWindowAttachment(u32 index) = 0;
            virtual Texture* GetDepthAttachment() = 0;

            u32 GetFrameWidth() { return width; };
            u32 GetFrameHeight() { return height; };

            virtual Texture* CreateTexture(TextureCreateInfo& info) = 0;
            virtual Material* CreateMaterial(MaterialCreateInfo& info) = 0;
            virtual Geometry* CreateGeometry(GeometryCreateInfo& info) = 0;
            virtual Shader* CreateShader(ShaderConfig& config) = 0;
            virtual Sampler* CreateSampler(SamplerCreateInfo info) = 0;
            virtual RenderTarget* CreateRenderTarget(RenderTargetCreateInfo& info) = 0;
            virtual Renderpass* CreateRenderpass(RenderpassCreateInfo& info) = 0;

        protected:
            std::string name;
            u32 width;
            u32 height;
            u32 cached_width;
            u32 cached_height;

        friend class RendererFrontend;
    };

    class ENGINE_API RendererFrontend {
        public: 
            RendererFrontend(RendererSetup* setup);
            ~RendererFrontend();

            static b8 Initialize(RendererSetup setup, RendererBackendType type);
            static void Shutdown();
            static RendererFrontend* GetInstance();
            static RendererBackend* GetBackend() { return instance->backend; };
            static b8 DrawFrame(RenderPacket* packet);
            static u32 GetFrameHeightS();
            static u32 GetFrameWidthS();

            void Resized(u16 width, u16 height);

            u32 GetFrameWidth() { return backend->GetFrameWidth(); };
            u32 GetFrameHeight() { return backend->GetFrameHeight(); };
            
            Texture* CreateTexture(TextureCreateInfo& info);
            Material* CreateMaterial(MaterialCreateInfo& info);
            Geometry* CreateGeometry(GeometryCreateInfo& info);
            Shader* CreateShader(ShaderConfig& config);
            Sampler* CreateSampler(SamplerCreateInfo info);
            RenderTarget* CreateRenderTarget(RenderTargetCreateInfo& info);
            Renderpass* CreateRenderpass(RenderpassCreateInfo& info);
            
            std::vector<Mesh*> meshes;

        private:
            b8 BeginFrame(f32 delta_time);
            b8 EndFrame(f32 delta_time);
            // b8 UpdateGlobalState(glm::mat4 projection, glm::mat4 view, glm::vec3 view_position, glm::vec4 ambient_colour, i32 mode);
            void DrawGeometry(GeometryRenderData data);
            
            void CreateEventListeners();
            RendererInitializationSetup CreateInitSetup();

            void InitializeRenderer();
            void GetRenderpasses();
            b8 RegenerateRenderTargets();

            b8 OnDebugEvent(EventType type, EventContext& context);
            b8 OnRegenerateRenderTargets(EventType type, EventContext& context);

            b8 CreateBackend(RendererSetup setup, RendererBackendType type);
            void ShutdownBackend();

            b8 _DrawFrame(RenderPacket* packet);

            void GetCameraSystem();

            static RendererFrontend* instance;
            RendererBackend* backend;

            Renderpass* world_renderpass;
            Renderpass* ui_renderpass;

            CameraSystem* camera_system;

            glm::vec4 ambient_color;
            u32 shader_debug_mode;

            u32 framebuffer_width;
            u32 framebuffer_height;
            std::atomic<b8> resizing;
            u32 frames_since_resizing;

            u32 image_count;

            const c8* world_renderpass_name = "Renderpass.Builtin.World";
            const c8* ui_renderpass_name = "Renderpass.Builtin.UI";
    };

};
#pragma once
#include "defines.hpp"
#include "render_target.hpp"

namespace Engine {

    enum class RenderpassClearFlag {
        CLEAR_NONE = 0x0,
        CLEAR_COLOR_BUFFER = 0x1,
        CLEAR_COLOR_DEPTH_BUFER = 0x2,
        CLEAR_COLOR_STENCIL_BUFFER = 0x4
    };

    ENABLE_BITMASK_OPERATORS(RenderpassClearFlag)

    struct RenderpassCreateInfo {
        std::string name;
        std::string prev_name;
        std::string next_name;
        glm::vec4 render_area;
        glm::vec4 clear_color;
        RenderpassClearFlag clear_flags;
    };

    class Renderpass {
        public:
            virtual ~Renderpass() = default;

            virtual b8 Begin() = 0;
            virtual b8 End() = 0;
            virtual void OnResize(glm::vec4 render_area) = 0;

            std::string GetName() { return name; }

            std::vector<RenderTarget*> render_targets;
            glm::vec4 render_area;
            glm::vec4 clear_color;
        protected:
            std::string name;
    };

}
#pragma once
#include "defines.hpp"
#include "resources/texture/texture.hpp"

namespace Engine {

    struct RenderTargetCreateInfo {
        std::string name;
        std::vector<Texture*> attachments;
        class Renderpass* renderpass;
        u32 width;
        u32 height;
        b8 manage_attachments;
    };

    class RenderTarget {
        public:
            virtual ~RenderTarget() = default;

            std::string GetName() { return name; }
        
        protected:
            std::string name;
            b8 sync_to_window_size;
            
            std::vector<Texture*> attachments;
    };

}
#pragma once

#include "defines.hpp"

namespace Engine {

    class ENGINE_API Resource {
        public:
            Resource(u32 loader_id, std::string name, std::string full_path);
            ~Resource();

        protected:
            u32 loader_id;
            std::string name;
            std::string full_path;
    };

}
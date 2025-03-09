#pragma once

#include "defines.hpp"
#include "systems/resource/resources/base/resource.hpp"

namespace Engine {

    class ENGINE_API BinaryResource : public Resource {
        public:
            BinaryResource(
                u32 loader_id, std::string name, 
                std::string full_path, std::vector<c8> data
            );
            ~BinaryResource();

            std::vector<c8> GetData() { return data; };

        protected:
            std::vector<c8> data;
    };

}
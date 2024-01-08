#pragma once

#include "defines.hpp"
#include "systems/resource/loaders/base/resource_loader.hpp"
#include "platform/filesystem.hpp"
#include "renderer/renderer_types.hpp"
#include "resources/geometry/geometry.hpp"

namespace Engine {
    
    enum class MeshFileType {
        NOT_FOUND,
        OBJ,
        E3DM
    };

    struct SupportedMeshFileType {
        std::string ext;
        MeshFileType type;
        b8 is_binary;
    };

    struct VertexIndexData {
        u32 position_idx;
        u32 normal_idx;
        u32 texcoord_idx;
    };

    struct MeshFaceData {
        VertexIndexData vertices[3];
    };

    struct MeshGroup {
        std::vector<MeshFaceData> faces;
    };

    class MeshLoader : public ResourceLoader {
        public:
            MeshLoader(u32 id, std::string type_path, std::string custom_type);
            MeshLoader(u32 id, std::string type_path);

            Resource* Load(std::string name);
        
        protected:
            Resource* LoadOBJ(const std::string& file_path, const std::string& name);
            b8 WriteToE3DM(const std::string& file_path, const std::string& name, GeometryConfigs& configs);

    };

} 
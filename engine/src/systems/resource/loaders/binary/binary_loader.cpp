#include "binary_loader.hpp"

#include "core/logger/logger.hpp"
#include "platform/filesystem.hpp"
#include "systems/resource/resources/binary/binary_resource.hpp"

namespace Engine {
    BinaryLoader::BinaryLoader(u32 id, std::string type_path, std::string custom_type) : ResourceLoader(id, ResourceType::BINARY, type_path, custom_type) {};
    BinaryLoader::BinaryLoader(u32 id, std::string type_path) : ResourceLoader(id, ResourceType::BINARY, type_path) {};

    Resource* BinaryLoader::Load(std::string name) {
        std::string file_name = StringFormat("%s/%s", type_path.c_str(), name.c_str());

        File* file = FileSystem::FileOpen(file_name, FileMode::READ, true);

        if (!file->IsReady()) {
            ERROR("Unable to read binary file: %s.", file_name.c_str());
            return nullptr;
        }

        std::vector<c8> bytes = file->ReadAllBytes();

        if (bytes.size() == 0) {
            ERROR("Unable to read binary file: %s.", file_name.c_str());
            return nullptr;
        }
        
        FileSystem::FileClose(file);

        return new BinaryResource(id, name, file_name, bytes);
    };
}

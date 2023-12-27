#include "filesystem.hpp"

#include <sys/stat.h>
#include <filesystem>

namespace Engine {

    std::vector<c8> File::ReadAllBytes() {
        if (!binary || !handle.is_open() || mode != FileMode::READ) {
            return std::vector<c8>();
        }
        std::ifstream::pos_type pos = handle.tellg();

        if (pos == 0) {
            return std::vector<c8>();
        }

        std::vector<c8>  result(pos);

        handle.seekg(0, std::ios::beg);
        handle.read(result.data(), pos);

        return result;
    };

    b8 File::WriteLine(std::string line) {
        if (binary || !handle.is_open() || mode != FileMode::WRITE) {
            return false;
        }
        handle << line;
        return true;
    };

    b8 File::ReadLine(std::string& line) {
        if (binary) {
            return false;
        }
        if (!std::getline(this->handle, line)) {
            return false;
        }
        return true;
    };

    std::vector<std::string> File::ReadAllLines() {
        if (binary) {
            return std::vector<std::string>();
        }
        std::string line_buffer;
        std::vector<std::string> lines_array;
        while (std::getline(this->handle, line_buffer)) {
            lines_array.push_back(line_buffer);
        };
        return lines_array;
    };

    File::File(std::string path, FileMode mode, b8 binary) {
        ready = false;
        this->mode = mode;
        this->binary = binary;

        if (mode == FileMode::READ && binary) {
            this->handle.open(path, std::ios::in | std::ios::binary | std::ios::ate);
        } else if (mode == FileMode::READ && !binary) {
            this->handle.open(path, std::ios::in);
        } else if (mode == FileMode::WRITE && binary) {
            this->handle.open(path, std::ios::binary | std::ios::out);
        } else if (mode == FileMode::WRITE && !binary) {
            this->handle.open(path, std::ios::app);
        }

        if (!this->handle.fail()) {
            ready = true;
        }
    }; 

    File::~File() {
        this->handle.close();
    };

    b8 FileSystem::FileExists(std::string path) {
        if (std::filesystem::exists(path)) { 
            return true;
        } else { 
            return false;
        };
    };

    File* FileSystem::FileOpen(std::string path, FileMode mode, b8 binary) {
        return new File(path, mode, binary);
    };

    void FileSystem::FileClose(File* file) {
        delete file;
    };

    tinyxml2::XMLDocument* FileSystem::OpenXml(std::string path) {
        tinyxml2::XMLDocument* doc = new tinyxml2::XMLDocument();
        doc->LoadFile(path.c_str());
        return doc;
    };

    void FileSystem::CloseXml(tinyxml2::XMLDocument* file) {
        delete file;
    };
};
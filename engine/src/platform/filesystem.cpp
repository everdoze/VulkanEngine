#include "filesystem.hpp"

#include <sys/stat.h>
#include <filesystem>

namespace Engine {

    std::vector<char> File::ReadAllBytes() {
        if (!binary || !handle.is_open() || mode != FileMode::READ) {
            return std::vector<char>();
        }
        std::ifstream::pos_type pos = handle.tellg();

        if (pos == 0) {
            return std::vector<char>();
        }

        std::vector<char>  result(pos);

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

    std::string File::ReadLine() {
        if (binary) {
            return nullptr;
        }
        std::string line;
        std::getline(this->handle, line);
        return line;
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

        // auto open_mode = binary ? std::fstream::binary | 
        //     (mode == FileMode::READ ? std::fstream::in : std::fstream::app) : 
        //     (mode == FileMode::READ ? std::fstream::in : std::fstream::app);
    
        // this->handle.open(path, open_mode);
        if (!this->handle.fail()) {
            ready = true;
        }
    }; 

    File::~File() {
        this->handle.close();
    };

    b8 FileSystem::FileExists(std::string path) {
        // #ifdef _MSC_VER
        //     struct _stat buffer;
        //     return _stat(path.c_str(), &buffer);
        // #else
        //     struct stat buffer;
        //     return stat(path.c_str(), &buffer) == 0;
        // #endif
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

};
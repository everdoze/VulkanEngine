#pragma once

#include "defines.hpp"
#include <fstream>
#include <iostream>

namespace Engine {

    typedef enum class FileMode {
        READ = 0x01,
        WRITE = 0x02
    } FileMode;

    class File {
        public:
            File(std::string path, FileMode mode, b8 binary);
            ~File();

            b8 ReadLine(std::string& line);
            std::vector<std::string> ReadAllLines();

            std::vector<c8> ReadAllBytes();

            b8 WriteLine(std::string line);

            b8 IsReady() { return ready; };
        private:
            FileMode mode;
            b8 binary;
            b8 ready;
            std::fstream handle;
    };

    class FileSystem {
        public:
            static b8 FileExists(std::string path);
            static File* FileOpen(std::string path, FileMode mode, b8 binary);
            static void FileClose(File* file);
        private:
            FileSystem();
    };

};
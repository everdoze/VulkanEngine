#include "logger.hpp"

#include "platform/platform.hpp"

namespace Engine {
    
    Logger* Logger::m_Instance = nullptr;
    
    Logger::Logger(std::string file_path) {
        file = nullptr;
        if (file_path.size()) {
            file = FileSystem::FileOpen(file_path, FileMode::WRITE, false);
        }
    };

    Logger::~Logger() {
        if (file) {
            file->WriteLine("-------------------END OF LOG---------------------\n");
            FileSystem::FileClose(file);
        }
    };

    Logger* Logger::Initialize() {
        if (!m_Instance) {
            m_Instance = new Logger(
                #ifdef _DEBUG
                LOG_FILE_PATH
                #else
                ""
                #endif
            );
        }
        return m_Instance;
    };
    
    void Logger::Shutdown() {
        delete m_Instance;
    };

    Logger* Logger::GetLogger() {
        return m_Instance;
    };

    std::string FormatLogLevel(std::string text, u8 level) {
        const char* log_level_names[6] = {
            "[FATAL]: ",
            "[ERROR]: ",
            "[WARN]: ",
            "[INFO]: ",
            "[DEBUG]: ",
            "[TRACE]: "
        };
        return StringFormat("%s%s\n", log_level_names[level], text.c_str());
    };

    void Logger::LogString(std::string text, u8 level) {
        std::string temp = FormatLogLevel(text, level);
        Platform::ConsoleWrite(temp, level);
        file->WriteLine(temp);
    };

    void Logger::LogError(std::string text, u8 level) {
        std::string temp = FormatLogLevel(text, level);
        Platform::ConsoleWriteError(temp, level);
        file->WriteLine(temp);
    };
    

    void ReportAssertionFailure(const char* expression, const char* message, const char* file, i32 line) {
        Logger::GetLogger()->FormatLog("Assertion failure: %s, message: '%s', in file: %s, line: %d", LogLevel::FATAL, expression, message, file, line);
    };
};
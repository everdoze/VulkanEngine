#pragma once

#include "core/utils/string.hpp"
#include "core/utils/asserts.hpp"
#include "defines.hpp"
#include "platform/filesystem.hpp"

namespace Engine {
    typedef enum LogLevel {
        FATAL = 0,
        ERROR = 1,
        WARN = 2,
        INFO = 3,
        DEBUG = 4,
        TRACE = 5
    } LogLevel;

    #define LOG_FILE_PATH "log.txt"

    class ENGINE_API Logger {
        public:
            Logger(std::string file_path);
            ~Logger();

            static Logger* Initialize();
            static void Shutdown();
            static Logger* GetLogger();
            
            template<typename... Args>
            static void Fatal(std::string text, const Args &... args) {
                Logger::GetLogger()->FormatLog(text, Engine::LogLevel::FATAL, args...);
            }

            template<typename... Args>
            static void Error(std::string text, const Args &... args) {
                Logger::GetLogger()->FormatLog(text, Engine::LogLevel::ERROR, args...);
            }

            template<typename... Args>
            static void Warn(std::string text, const Args &... args) {
                Logger::GetLogger()->FormatLog(text, Engine::LogLevel::WARN, args...);
            }

            template<typename... Args>
            static void Info(std::string text, const Args &... args) {
                Logger::GetLogger()->FormatLog(text, Engine::LogLevel::INFO, args...);
            }

            template<typename... Args>
            static void Debug(std::string text, const Args &... args) {
                Logger::GetLogger()->FormatLog(text, Engine::LogLevel::DEBUG, args...);
            }

            template<typename... Args>
            static void Trace(std::string text, const Args &... args) {
                Logger::GetLogger()->FormatLog(text, Engine::LogLevel::TRACE, args...);
            }

            template<typename... Args>
            void FormatLog(std::string text, u8 level, const Args &... args) {
                std::string log_message = StringFormat(text, args ...);
                if (level < 2) {
                    LogError(log_message, level);
                } else {
                    LogString(log_message, level);
                }
            }
        private:
            static Logger* m_Instance;
            void LogString(std::string text, u8 level);
            void LogError(std::string text, u8 level);
            File* file;
    };
};

#define LOG(text, color, ...) Logger::GetLogger()->FormatLog(text, color, __VA_ARGS__);



#ifdef _DEBUG
#define FATAL(text, ...) Engine::Logger::Fatal(text, __VA_ARGS__);
#define ERROR(text, ...) Engine::Logger::Error(text, __VA_ARGS__);
#define WARN(text, ...) Engine::Logger::Warn(text, __VA_ARGS__);
#define INFO(text, ...) Engine::Logger::Info(text, __VA_ARGS__);
#define DEBUG(text, ...) Engine::Logger::Debug(text, __VA_ARGS__);
#define TRACE(text, ...) Engine::Logger::Trace(text, __VA_ARGS__);
#else
#define FATAL(text, ...);
#define ERROR(text, ...);
#define WARN(text, ...);
#define INFO(text, ...);
#define DEBUG(text, ...);
#define TRACE(text, ...);
#endif

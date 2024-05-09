#pragma once

#include "defines.hpp"

namespace Engine {
    class ENGINE_API Platform {
        public:
            static b8 Initialize(std::string name, i32 x, i32 y, i32 width, i32 height);
            static void Shutdown();

            static void ConsoleWriteError(std::string& text, u8 color);
            static void ConsoleWrite(std::string& text, u8 color);
            static f64 GetAbsoluteTime();
            static b8 ClockSetup();
            static void Sleep(u64 ms);
            static std::vector<char*> GetRequiredExtensionsVK();

            static b8 SetCursorPosition(i32 x, i32 y);
            static void CursorVisibility(b8 show);

            static b8 CreateVulkanSurface(class VulkanRendererBackend* backend);
            static void DestroyVulkanSurface(class VulkanRendererBackend* backend);

            static b8 PumpMessages();

            static void ZMemory(void* block, u64 size);
            static void CMemory(void* dest, const void* source, u64 size);
            static void FMemory(void* block);
            static void* AMemory(u64 size);
            static void SMemory(void* block, i32 data, u64 size);
    };  
};
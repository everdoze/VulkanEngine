#include "platform.hpp"

#include "core/logger/logger.hpp"
#include "core/event/event.hpp"
#include "core/input/input.hpp"

#ifdef PLATFORM_WINDOWS

#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>
#include <time.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include "renderer/backend/vulkan/vulkan.hpp"

namespace Engine {

    static f64 clock_frequency;
    static LARGE_INTEGER start_time;

    class Win32PlatformState {
        public:
            HINSTANCE h_instance;
            HWND hwnd;
            VkSurfaceKHR surface;
            u32 window_style;
            u32 window_ex_style;
            RECT border_rect;
            b8 message_pumping = false;
    };
    static Win32PlatformState* w32State = nullptr;


    LRESULT CALLBACK Win32ProcessMessage(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param);

    Keys SplitCodeLR(b8 pressed, i32 in_left, i32 in_right, Keys out_left, Keys out_right) {
        InputSystem* input_system = InputSystem::GetInstance();

        Keys key;
        if ((GetKeyState(in_left) & 0x8000) || (!pressed && input_system->IsKeyDown(out_left))) {
            key = out_left;
        } else if ((GetKeyState(in_right) & 0x8000) || (!pressed && input_system->IsKeyDown(out_right))) {
            key = out_right;
        }
        return key;
    }

    b8 Platform::Initialize(std::string name, i32 x, i32 y, i32 width, i32 height) {
        if (!w32State) {
            w32State = new Win32PlatformState();
        }

        w32State->h_instance = GetModuleHandle(0);
        HICON icon = LoadIcon(w32State->h_instance, IDI_APPLICATION);
        WNDCLASSA wc;
        memset(&wc, 0, sizeof(wc));
        wc.style = CS_DBLCLKS;
        wc.lpfnWndProc = Win32ProcessMessage;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = w32State->h_instance;
        wc.hIcon = icon;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = NULL;
        wc.lpszClassName = "engine_window";

         if (!RegisterClassA(&wc)) {
            MessageBoxA(0, "Window registration failed", "Error!", MB_ICONEXCLAMATION | MB_OK);
            return false;
        }

        u32 client_x = x;
        u32 client_y = y;
        u32 client_width = width;
        u32 client_height = height;

        u32 window_x = client_x;
        u32 window_y = client_y;
        u32 window_width = client_width;
        u32 window_height = client_height;

        srand(time(NULL));

        w32State->window_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
        w32State->window_ex_style = WS_EX_APPWINDOW;

        w32State->window_style |= WS_MAXIMIZEBOX;
        w32State->window_style |= WS_MINIMIZEBOX;
        w32State->window_style |= WS_THICKFRAME;

        RECT* brect = &w32State->border_rect;
        AdjustWindowRectEx(brect, w32State->window_style, 0, w32State->window_ex_style);

        window_x += brect->left;
        window_y += brect->top;

        window_width += brect->right - brect->left;
        window_height += brect->bottom - brect->top;

        HWND handle = CreateWindowExA(
            w32State->window_ex_style, "engine_window", name.c_str(),
            w32State->window_style, window_x, window_y, window_width, window_height,
            0, 0, w32State->h_instance, 0);

        if (handle == 0) {
            MessageBoxA(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
            FATAL("Window creation failed!");
            return false;
        } else {
            w32State->hwnd = handle;
        }


        //Show window
        b32 should_activate = true;
        i32 show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;
        ShowWindow(w32State->hwnd, show_window_command_flags);

        //Clock
        ClockSetup();

        DEBUG("Platform successfully initialized.");

        return true;
    };
    
    b8 Platform::PumpMessages() {
        if (w32State) {
            MSG message;
            while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&message);
                DispatchMessageA(&message);
            }

            return true;
        }
        return false;
    }

    void Platform::Shutdown() {
        DEBUG("Shutting down Platform.");
        delete w32State;
    };

    void ConsoleWriteMessage(std::string& text, u8 color, b8 error = false) {
        HANDLE console_handle;
        if (error) {
            console_handle = GetStdHandle(STD_ERROR_HANDLE);
        } else {
            console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        }
        // FATAL, ERROR, WARN, INFO, DEBUG, TRACE
        static u8 levels[6] = {64, 4, 6, 2, 1, 8};
        SetConsoleTextAttribute(console_handle, levels[color]);

        OutputDebugStringA(text.c_str());
        u64 length = strlen(text.c_str());
        LPDWORD number_written = 0;
        WriteConsoleA(console_handle, text.c_str(), (DWORD)length, number_written, 0);
    };

    void Platform::ConsoleWrite(std::string& text, u8 color) {
        ConsoleWriteMessage(text, color, false);
    };

    void Platform::ConsoleWriteError(std::string& text, u8 color) {
        ConsoleWriteMessage(text, color, true);
    };

    /// @brief Setup system clock
    /// @return true if success
    b8 Platform::ClockSetup() {
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        clock_frequency = 1.0 / (f64)frequency.QuadPart;
        QueryPerformanceCounter(&start_time);
        return true;
    };

    f64 Platform::GetAbsoluteTime() {
        LARGE_INTEGER now_time;
        QueryPerformanceCounter(&now_time);
        return (f64)now_time.QuadPart * clock_frequency;
    };

    void Platform::PSleep(u64 ms) {
        Sleep(ms);
    };

    std::vector<char*> Platform::GetRequiredExtensionsVK() {
        std::vector<char*> extensions;
        extensions.push_back((char*)"VK_KHR_win32_surface");
        return extensions;
    };

    b8 Platform::CreateVulkanSurface(VulkanRendererBackend* backend) {
        if (!w32State) {
            return false;
        }

        VkWin32SurfaceCreateInfoKHR create_info = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
        create_info.hinstance = w32State->h_instance;
        create_info.hwnd = w32State->hwnd;

        VkResult result = vkCreateWin32SurfaceKHR(backend->GetVulkanInstance(), &create_info, backend->GetVulkanAllocator(), &w32State->surface);
        if (result != VK_SUCCESS) {
            FATAL("vkCreateWin32SurfaceKHR call failed.");
            return false;
        }

        backend->SetVulkanSurface(w32State->surface);

        return true;
    };

    void Platform::DestroyVulkanSurface(class VulkanRendererBackend* backend) {
        vkDestroySurfaceKHR(backend->GetVulkanInstance(), backend->GetVulkanSurface(), backend->GetVulkanAllocator());
        backend->SetVulkanSurface(nullptr);
        w32State->surface = nullptr;
    };

    LRESULT CALLBACK Win32ProcessMessage(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param) {
        switch(msg) {
            case WM_ERASEBKGND: {
                return true;
            } break;
                

            case WM_CLOSE: {
                EventSystem::GetInstance()->FireEvent(EventType::AppQuit, {});
                return true;
            } break;
                

            case WM_DESTROY: {
                PostQuitMessage(0);
                return false;
            } break;
                

            case WM_SIZE: {
                // Get the updated size.
                RECT r;
                GetClientRect(hwnd, &r);
                u32 width = r.right - r.left;
                u32 height = r.bottom - r.top;

                EventContext context = {};
                context.data.u16[0] = (u16)width;
                context.data.u16[1] = (u16)height;
                EventSystem::GetInstance()->FireEvent(EventType::WindowResize, context);
            } break;

            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYUP: {
                // Key pressed || released
                b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
                Keys key = static_cast<Keys>(w_param);
            
                if (w_param == VK_MENU) {
                    key = SplitCodeLR(pressed, VK_LMENU, VK_RMENU, Keys::KEYBOARD_LALT, Keys::KEYBOARD_RALT);
                } 
                if (w_param == VK_SHIFT) {
                    key = SplitCodeLR(pressed, VK_LSHIFT, VK_RSHIFT, Keys::KEYBOARD_LSHIFT, Keys::KEYBOARD_RSHIFT);
                }
                if (w_param == VK_CONTROL) {
                    key = SplitCodeLR(pressed, VK_LCONTROL, VK_RCONTROL, Keys::KEYBOARD_LCONTROL, Keys::KEYBOARD_RCONTROL);
                }

                InputSystem::GetInstance()->ProcessKey(key, pressed);
            } break;
            case WM_MOUSEMOVE: {
                // //Mouse move
                i32 x_position = GET_X_LPARAM(l_param);
                i32 y_position = GET_Y_LPARAM(l_param);

                InputSystem::GetInstance()->ProcessMouseMove(x_position, y_position);
            } break;
            case WM_MOUSEWHEEL: {
                i32 z_delta = GET_WHEEL_DELTA_WPARAM(w_param);
                if (z_delta != 0) {
                    //Flatten the input to an OS-independent (-1, 1)
                    z_delta = (z_delta < 0) ? -1 : 1;
                    
                    InputSystem::GetInstance()->ProcessMouseWheel(z_delta);
                }
            } break;
            case WM_LBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_MBUTTONUP:
            case WM_RBUTTONUP: {
                b8 pressed = msg == WM_LBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_RBUTTONDOWN;
                Buttons mouse_button = Buttons::MAX_BUTTONS;
                switch (msg) {
                    case WM_LBUTTONDOWN:
                    case WM_LBUTTONUP:
                        mouse_button = Buttons::LEFT;
                        break;
                    case WM_MBUTTONDOWN:
                    case WM_MBUTTONUP:
                        mouse_button = Buttons::MIDDLE;
                        break;
                    case WM_RBUTTONDOWN:
                    case WM_RBUTTONUP:
                        mouse_button = Buttons::RIGHT;
                        break;
                }

                // Pass over to the input subsystem.
                if (mouse_button != Buttons::MAX_BUTTONS) {
                    InputSystem::GetInstance()->ProcessButton(mouse_button, pressed);
                }
            } break;
        }

        return DefWindowProcA(hwnd, msg, w_param, l_param);
    };

    void Platform::CursorVisibility(b8 show) {
        ShowCursor(show);
    };

    void FastSetCursorPos(int x, int y) {
        INPUT input;
        input.type = INPUT_MOUSE;
        input.mi.dx = x * (65536 / GetSystemMetrics(SM_CXSCREEN));
        input.mi.dy = y * (65536 / GetSystemMetrics(SM_CYSCREEN));
        input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
        SendInput(1, &input, sizeof(INPUT));
    }

    b8 Platform::SetCursorPosition(i32 x, i32 y) {
        RECT clientRect;
        GetClientRect(w32State->hwnd, &clientRect);

        POINT topLeft = { clientRect.left, clientRect.top };

        ClientToScreen(w32State->hwnd, &topLeft);

        i32 absolute_X = x + (topLeft.x - w32State->border_rect.left);
        i32 absolute_Y = y + (topLeft.y - w32State->border_rect.top);

        FastSetCursorPos(absolute_X, absolute_Y);
        InputSystem::GetInstance()->ProcessMouseMove(x, y);
        return true;
    };

    void Platform::ZrMemory(void* block, u64 size) {
        memset(block, 0, size);
    };

    void Platform::FrMemory(void* block) {
        free(block);
    };

    void Platform::CpMemory(void* dest, const void* source, u64 size) {
        memcpy(dest, source, size);
    };

    void* Platform::AllocMemory(u64 size) {
        return malloc(size);
    };

    void Platform::SetMemory(void* block, i32 data, u64 size) {
        memset(block, data, size);
    };

};

#endif
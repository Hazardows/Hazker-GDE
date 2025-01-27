#include "platform/platform.h"

// Windows platform layer
#if HPLATFORM_WINDOWS

#include "core/logger.h"
#include "core/input.h"
#include "core/events.h"
#include "containers/darray.h"

#include <windows.h>
#include <windowsx.h> // param input extraction

// For surface creation
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include "renderer/vulkan/vk_types.inl" 

typedef struct platform_state {
    HINSTANCE hInstance;
    HWND hWnd;
    VkSurfaceKHR surface;
} platform_state;

// Clock
static f64 clock_frequency;
static LARGE_INTEGER start_time;

static platform_state* state_ptr;

LRESULT CALLBACK win32_process_message(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void clock_setup() {
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clock_frequency = 1.0 / (f64)frequency.QuadPart;
    QueryPerformanceCounter(&start_time);
}

b8 platformStartup(u64* memory_requirement, void* state, const char* appName, i32 x, i32 y, i32 width, i32 height) {
    *memory_requirement = sizeof(platform_state);
    if (state == NULL) {
        return true;
    }
    state_ptr = state;
    state_ptr->hInstance = GetModuleHandleA(0);

    // Setup and register window class
    HICON icon = LoadIcon(state_ptr->hInstance, IDI_APPLICATION);
    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.style = CS_DBLCLKS; // Get double clicks
    wc.lpfnWndProc = win32_process_message;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = state_ptr->hInstance;
    wc.hIcon = icon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); // Manage the cursor manually
    wc.hbrBackground = NULL; // Transparent
    wc.lpszClassName = "Hazker_Window_Class";

    if (!RegisterClassA(&wc)) {
        MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    // Create window
    u32 clientX = x;
    u32 clientY = y;
    u32 clientWidth = width;
    u32 clientHeight = height;

    u32 windowX = clientX;
    u32 windowY = clientY;
    u32 windowWidth = clientWidth;
    u32 windowHeight = clientHeight;

    u32 windowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
    u32 windowExStyle = WS_EX_APPWINDOW;

    windowStyle |= WS_MINIMIZEBOX;
    // Window resizing, disabled because of exceptions on resize
    // windowStyle |= WS_MAXIMIZEBOX;
    // windowStyle |= WS_THICKFRAME;


    // Obtain the size of the border
    RECT borderRect = { 0, 0, 0, 0 };
    AdjustWindowRectEx(&borderRect, windowStyle, 0, windowExStyle);

    // In this case, the border rectangle is negative
    windowX += borderRect.left;
    windowY += borderRect.top;

    // Grow by the size of the OS border
    windowWidth += borderRect.right - borderRect.left;
    windowHeight += borderRect.bottom - borderRect.top;

    // TODO: Make this configurable (fullscreen/windowed)
    HWND handle = CreateWindowExA(
        windowExStyle, "Hazker_Window_Class", appName,
        windowStyle, windowX, windowY, windowWidth, windowHeight,
        0, 0, state_ptr->hInstance, 0
    );
    // Create window (fullscreen)
    //HWND handle = CreateWindowExA( 
    //    WS_EX_APPWINDOW, "Hazker_Window_Class", appName, 
    //    WS_POPUP | WS_VISIBLE, // WS_POPUP for fullscreen 
    //    0, 0, GetSystemMetrics(SM_CXSCREEN),
    //    GetSystemMetrics(SM_CYSCREEN), 0, 0, 
    //    state->hInstance, 0
    //);

    if (handle == NULL) {
        MessageBoxA(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        HFATAL("Window creation failed!");
        return false;
    } else {
        state_ptr->hWnd = handle;
    }

    // Show the window
    b32 shouldActivate = 1; // TODO: if the window should not accept input, this should be false
    // TODO: Make this configurable
    i32 showWindowCommandFlags = shouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;
    //If initially minimized, use SW_MINIMIZE :SW_SHOWMINNOACTIVE
    //If initially maximized, use SW_SHOWMAXIMIZED : SW_MAXIMIZE

    ShowWindow(state_ptr->hWnd, showWindowCommandFlags);

    //Clock setup
    clock_setup();

    return true;
}

void platformShutdown(void* pState) {
    if (state_ptr && state_ptr->hWnd) {
        DestroyWindow(state_ptr->hWnd);
        state_ptr->hWnd = NULL;
    }
}

b8 platformPumpMessages() {
    MSG message;
    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
    return true;
}

void* platformAllocate(u64 size, b8 aligned) {
    return malloc(size);
}

void platformFree(void* block, b8 aligned) {
    free(block);
}

void* platformZeroMemory(void* block, u64 size) {
    return memset(block, 0, size);
}

void* platformCopyMemory(void* dest, const void* source, u64 size) {
    return memcpy(dest, source, size);
}

void* platformSetMemory(void* dest, i32 value, u64 size) {
    return memset(dest, value, size);
}

void platformConsoleWrite(const char* message, u8 colour) {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    //FATAL, ERROR, WARNING, INFO, DEBUG, TRACE
    static u8 levels[6] = { 64, 4, 6, 2, 1, 8 };
    SetConsoleTextAttribute(consoleHandle, levels[colour]);
    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD numberWritten = 0;
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, numberWritten, 0);
}

void platformConsoleWriteError(const char* message, u8 colour) {
    HANDLE consoleHandle = GetStdHandle(STD_ERROR_HANDLE);
    //FATAL, ERROR, WARNING, INFO, DEBUG, TRACE
    static u8 levels[6] = { 64, 4, 6, 2, 1, 8 };
    SetConsoleTextAttribute(consoleHandle, levels[colour]);
    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD numberWritten = 0;
    WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length, numberWritten, 0);
}

f64 platformGetAbsoluteTime() {
    if (!clock_frequency) {
        clock_setup();
    }

    LARGE_INTEGER now_time;
    QueryPerformanceCounter(&now_time);
    return (f64)now_time.QuadPart * clock_frequency;
}

void platformSleep(u64 ms) {
    Sleep(ms);
}

void platformGetRequiredExtensionNames(const char*** names_darray) {
    darray_push(*names_darray, &"VK_KHR_win32_surface");
}

// Surface creation for Vulkan
b8 platformCreateVulkanSurface(vulkanContext* context) {
    if (!state_ptr) {
        return false;
    }
    
    VkWin32SurfaceCreateInfoKHR createInfo = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    createInfo.hinstance = state_ptr->hInstance;
    createInfo.hwnd = state_ptr->hWnd;

    VkResult result = vkCreateWin32SurfaceKHR(context->instance, &createInfo, context->allocator, &state_ptr->surface);
    if (result != VK_SUCCESS) {
        HFATAL("Vulkan Surface Creation Failed.");
        return false;
    }

    context->surface = state_ptr->surface;
    return true;
}

// Define the window procedure function
LRESULT CALLBACK win32_process_message(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_ERASEBKGND:
            // Notify the OS that erasing will be handled by the application to prevent flicker
            return 1;
        case WM_CLOSE:
            // Fire an event for the application to quit
            eventContext data = {};
            eventFire(EVENT_CODE_APPLICATION_QUIT, 0, data);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE: {
            //Get the updated size
            RECT r;
            GetClientRect(hWnd, &r);
            u32 width = r.right - r.left;
            u32 height = r.bottom - r.top;
            
            // Fire the event. The application layer should pick this up, but not handle it
            // as it should be visible to other parts of the application.
            eventContext context;
            context.data.u16[0] = (u16)width;
            context.data.u16[1] = (u16)height;
            eventFire(EVENT_CODE_RESIZED, 0, context);
        } break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            //Key pressed/released
            b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
            keys key = (u16)wParam;

            // Check for extended scan code.
            b8 is_extended = (HIWORD(lParam) & KF_EXTENDED) == KF_EXTENDED;

            if (wParam == VK_MENU) {
                key = is_extended ? KEY_RALT : KEY_LALT;
            }
            else if (wParam == VK_SHIFT) {
                // KF_EXTENDED is not set for shift keys
                u32 left_shift = MapVirtualKey(VK_LSHIFT, MAPVK_VK_TO_VSC);
                u32 scancode = ((lParam & (0xFF << 16)) >> 16);
                key = scancode == left_shift ? KEY_LSHIFT : KEY_RSHIFT;
            }
            else if (wParam == VK_CONTROL) {
                key = is_extended ? KEY_RCONTROL : KEY_LCONTROL;
            }

            // Pass to the input subsystem for processing
            input_process_key(key, pressed);
            
            // Return 0 to prevent default window behaviour for some keypresses, such as alt.
            return 0;
        }
        case WM_MOUSEMOVE: {
            //Mouse move
            i32 xPos = GET_X_LPARAM(lParam);
            i32 yPos = GET_Y_LPARAM(lParam);
            
            // Pass to the input subsystem for processing
            input_process_mouse_move(xPos, yPos);
        } break;
        case WM_MOUSEWHEEL: {
            i32 zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            if (zDelta != 0) {
                // Flatten the input to an OS independient (-1, 1)
                zDelta = (zDelta < 0) ? -1 : 1;
                // Pass to the input subsystem for processing
                input_process_mouse_wheel(zDelta);
            }
        } break;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            b8 pressed = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
            buttons mouseButton = BUTTON_MAX_BUTTONS;
            switch (msg) {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                    mouseButton = BUTTON_LEFT;
                    break;
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                    mouseButton = BUTTON_MIDDLE;
                    break;
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                    mouseButton = BUTTON_RIGHT;
                    break;
            }

            // Pass to the input subsystem for processing
            if (mouseButton != BUTTON_MAX_BUTTONS) {
                input_process_button(mouseButton, pressed);
            }
        } break;
    }
    
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

#endif
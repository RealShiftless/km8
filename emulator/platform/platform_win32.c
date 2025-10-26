#include "platform/platform.h"

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

#include <stdlib.h>

typedef struct Km8PlatformWindow {
    HWND hwnd;
    bool should_close;
    BITMAPINFO bitmap_info;
} Km8PlatformWindow;

static HINSTANCE gInstance;
static ATOM gWindowClass;
static const wchar_t* gWindowClassName = L"KM8WindowClass";

static LRESULT CALLBACK km8_win32_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    Km8PlatformWindow* window = (Km8PlatformWindow*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    switch (msg) {
        case WM_NCCREATE: {
            CREATESTRUCTW* create = (CREATESTRUCTW*)lparam;
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)create->lpCreateParams);
            return DefWindowProcW(hwnd, msg, wparam, lparam);
        }

        case WM_CLOSE:
            if (window) {
                window->should_close = true;
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
}

static bool km8_win32_register_window_class(void) {
    if (gWindowClass != 0) {
        return true;
    }

    WNDCLASSEXW cls = {0};
    cls.cbSize        = sizeof(cls);
    cls.style         = CS_HREDRAW | CS_VREDRAW;
    cls.lpfnWndProc   = km8_win32_wndproc;
    cls.hInstance     = gInstance;
    cls.hCursor       = LoadCursor(NULL, IDC_ARROW);
    cls.lpszClassName = gWindowClassName;

    gWindowClass = RegisterClassExW(&cls);
    return gWindowClass != 0;
}

bool km8_platform_init(void) {
    gInstance = GetModuleHandleW(NULL);
    return km8_win32_register_window_class();
}

void km8_platform_shutdown(void) {
    if (gWindowClass != 0) {
        UnregisterClassW(gWindowClassName, gInstance);
        gWindowClass = 0;
    }
}

Km8PlatformWindow* km8_platform_window_create(const Km8PlatformWindowDesc* desc) {
    if (!km8_win32_register_window_class() || !desc) {
        return NULL;
    }

    Km8PlatformWindow* window = (Km8PlatformWindow*)calloc(1, sizeof(Km8PlatformWindow));
    if (!window) {
        return NULL;
    }

    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    if (desc->resizable) {
        style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
    }

    RECT rect = {0, 0, (LONG)desc->width, (LONG)desc->height};
    AdjustWindowRect(&rect, style, FALSE);

    wchar_t wtitle[256];
    int title_len = desc->title ? MultiByteToWideChar(CP_UTF8, 0, desc->title, -1, wtitle, 256) : 0;
    if (title_len <= 0) {
        lstrcpyW(wtitle, L"km8");
    }

    HWND hwnd = CreateWindowExW(
        0,
        gWindowClassName,
        wtitle,
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL,
        NULL,
        gInstance,
        window);

    if (!hwnd) {
        free(window);
        return NULL;
    }

    window->hwnd = hwnd;
    window->bitmap_info.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    window->bitmap_info.bmiHeader.biWidth       = (LONG)desc->width;
    window->bitmap_info.bmiHeader.biHeight      = -(LONG)desc->height; // top-down
    window->bitmap_info.bmiHeader.biPlanes      = 1;
    window->bitmap_info.bmiHeader.biBitCount    = 32;
    window->bitmap_info.bmiHeader.biCompression = BI_RGB;

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    return window;
}

void km8_platform_window_destroy(Km8PlatformWindow* window) {
    if (!window) {
        return;
    }

    if (window->hwnd) {
        DestroyWindow(window->hwnd);
        window->hwnd = NULL;
    }

    free(window);
}

void km8_platform_poll_events(void) {
    MSG msg;
    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

bool km8_platform_window_should_close(const Km8PlatformWindow* window) {
    return window ? window->should_close : true;
}

void km8_platform_window_present(Km8PlatformWindow* window,
                                 const void* pixels,
                                 uint32_t pitch,
                                 uint32_t width,
                                 uint32_t height) {
    if (!window || !window->hwnd || !pixels) {
        return;
    }

    if (pitch != width * 4) {
        return;
    }

    HDC dc = GetDC(window->hwnd);
    if (!dc) {
        return;
    }

    RECT client;
    GetClientRect(window->hwnd, &client);
    int client_width  = client.right - client.left;
    int client_height = client.bottom - client.top;

    window->bitmap_info.bmiHeader.biWidth  = (LONG)width;
    window->bitmap_info.bmiHeader.biHeight = -(LONG)height;

    StretchDIBits(dc,
                  0,
                  0,
                  client_width,
                  client_height,
                  0,
                  0,
                  (int)width,
                  (int)height,
                  pixels,
                  &window->bitmap_info,
                  DIB_RGB_COLORS,
                  SRCCOPY);

    ReleaseDC(window->hwnd, dc);
}

char* km8_platform_open_rom_dialog(void) {
    wchar_t file_buffer[MAX_PATH] = L"";
    wchar_t filter[] = L"KM8 ROM (*.bin)\0*.bin\0All Files (*.*)\0*.*\0\0";

    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize  = sizeof(ofn);
    ofn.hInstance    = gInstance;
    ofn.lpstrFilter  = filter;
    ofn.lpstrFile    = file_buffer;
    ofn.nMaxFile     = MAX_PATH;
    ofn.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    ofn.lpstrDefExt  = L"bin";
    ofn.lpstrTitle   = L"Select KM8 ROM";

    if (GetOpenFileNameW(&ofn)) {
        int utf8_len = WideCharToMultiByte(CP_UTF8, 0, file_buffer, -1, NULL, 0, NULL, NULL);
        if (utf8_len <= 0) {
            return NULL;
        }

        char* result = (char*)malloc((size_t)utf8_len);
        if (!result) {
            return NULL;
        }

        WideCharToMultiByte(CP_UTF8, 0, file_buffer, -1, result, utf8_len, NULL, NULL);
    return result;
}

#endif /* _WIN32 */

    return NULL;
}

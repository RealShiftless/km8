#include "platform.h"



static HWND hwnd;
static BITMAPINFO bmi;

uint32_t* gFramebuffer;
Game* gGame;

bool isRunning = false;

static double g_ns_per_cycle = 0.0;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT) {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            return TRUE;
        }
        break;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        gGame->KeyEvent((uint8_t)wParam, 1);
        break;

    case WM_KEYUP:
    case WM_SYSKEYUP:
        gGame->KeyEvent((uint8_t)wParam, 0);
        break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void WindowRenderFrame() {
    HDC hdc = GetDC(hwnd);

    RECT client;
    GetClientRect(hwnd, &client);
    int winWidth = client.right - client.left;
    int winHeight = client.bottom - client.top;

    SetStretchBltMode(hdc, COLORONCOLOR);
    StretchDIBits(
        hdc, 0, 0, winWidth, winHeight,
        0, 0, FRAMEBUFFER_W, FRAMEBUFFER_H,
        gFramebuffer, &bmi, DIB_RGB_COLORS, SRCCOPY
    );

    ReleaseDC(hwnd, hdc);
}

void WindowMain() {
    // Set the window to be running
    isRunning = true;


    gGame->Init();
    WindowRenderFrame();

    

    // Now loop until shi not running
    while (1) {
        // Do window messages
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                isRunning = false;
                return;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        gGame->UpdateFrame(0);
    }
}

void PlatformInit(HINSTANCE hInstance, int nCmdShow, Game* game) {
	gGame = game;

    // Init timing
    uint64_t t0 = __rdtsc();
    Sleep(100); // or use a precise sleep like a busy wait if you're wilding
    uint64_t t1 = __rdtsc();

    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);

    // Elapsed time in nanoseconds:
    double elapsed_ns = (100.0 * 1e6); // 100 ms = 100 million ns
    uint64_t elapsed_cycles = t1 - t0;

    g_ns_per_cycle = elapsed_ns / (double)elapsed_cycles;

    // Register a simple window class
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"PixelWindowClass";

    RegisterClass(&wc);

    // Get the actual size of the window
    RECT clientRect = { 0, 0, WINDOW_CLIENT_W, WINDOW_CLIENT_H };
    AdjustWindowRectEx(&clientRect, WS_OVERLAPPEDWINDOW, FALSE, 0);


    int windowWidth = clientRect.right - clientRect.left;
    int windowHeight = clientRect.bottom - clientRect.top;

    // Create a window using the registered class
    hwnd = CreateWindowEx(
        0, wc.lpszClassName, L"shiftless KM-8",
        WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowWidth, windowHeight,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, nCmdShow);

    // Configure bitmap header for a 32-bit top-down DIB
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = FRAMEBUFFER_W;
    bmi.bmiHeader.biHeight = -FRAMEBUFFER_H;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    // Create a DIB section and obtain a pointer to its pixel memory
    HDC screen = GetDC(NULL);
    HBITMAP dib = CreateDIBSection(
        screen, &bmi, DIB_RGB_COLORS, (void**)&gFramebuffer, NULL, 0
    );
    ReleaseDC(NULL, screen);

    if (gFramebuffer == NULL) {
        MessageBoxA(0, "Failed to create framebuffer", "Fatal", MB_OK | MB_ICONERROR);
        ExitProcess(1);
    }

    if (!hwnd) {
        MessageBoxA(0, "Failed to create window", "Fatal", MB_OK | MB_ICONERROR);
        ExitProcess(1);
    }

    // Go into the main loop
    WindowMain();
}

void PlatformExit() {
    isRunning = false;
}

uint64_t now_nanos() {
    return (uint64_t)(__rdtsc() * g_ns_per_cycle);
}
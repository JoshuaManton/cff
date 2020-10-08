#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct Window {
    HWND handle;
    HDC windows_device_context;

    int width;
    int height;
};

Window window;
float time_at_startup;

float time_now() {
    FILETIME ft = {};
    GetSystemTimeAsFileTime(&ft);
    double t = (f64)(((u64)ft.dwLowDateTime) | (((u64)ft.dwHighDateTime) << 32));
    return (t / 10.0 / 1000.0 / 1000.0) - time_at_startup;
}

LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM w, LPARAM l) {
    switch (uMsg) {
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(hwnd, uMsg, w, l);
}

void create_window(int width, int height) {
    const char *CLASS_NAME = "my window class";
    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW; // todo(josh): maybe remove HREDRAW and VREDRAW since we dont use WM_PAINT
    wc.hCursor = LoadCursorA(nullptr, IDC_ARROW);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = CLASS_NAME;
    auto c = RegisterClassExA(&wc);
    assert(c != 0);

    window.handle = CreateWindowEx(
        0,
        CLASS_NAME,
        "Fancy Window",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        300, 150, width, height,
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr
        );

    assert(window.handle != nullptr && "window.handle was null");
    window.windows_device_context = GetDC(window.handle);

    window.width = width;
    window.height = height;
}

void update_window() {
    MSG msg = {};
    while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}
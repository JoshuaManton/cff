#include "window.h"

double time_now() {
    FILETIME file_time = {};
    GetSystemTimeAsFileTime(&file_time);
    i64 t = (i64)(((u64)file_time.dwLowDateTime) | ((u64)file_time.dwHighDateTime) << 32);
    return (double)(t - 0x019db1ded53e8000) / 10000000.0;
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

Window create_window(int width, int height) {
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

    Window window = {};
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
    return window;
}

void update_window() {
    MSG msg = {};
    while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}
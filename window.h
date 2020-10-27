#pragma once

#include "basic.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct Window {
    HWND handle;
    HDC windows_device_context;

    int width;
    int height;
};

float time_now();
Window create_window(int width, int height);
void update_window();
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "math.h"
#include "basic.h"

void init_platform();

enum Input {
    INPUT_NONE,

    INPUT_MOUSE_LEFT,
    INPUT_MOUSE_RIGHT,
    INPUT_MOUSE_MIDDLE,

    INPUT_BACKSPACE,
    INPUT_TAB,

    INPUT_CLEAR, // ?
    INPUT_ENTER,

    INPUT_SHIFT,
    INPUT_CONTROL,
    INPUT_ALT,
    INPUT_PAUSE,
    INPUT_CAPS_LOCK,

    INPUT_ESCAPE,
    INPUT_SPACE,
    INPUT_PAGE_UP,
    INPUT_PAGE_DOWN,
    INPUT_END,
    INPUT_HOME,

    INPUT_UP,
    INPUT_DOWN,
    INPUT_LEFT,
    INPUT_RIGHT,

    INPUT_SELECT, // ?
    INPUT_PRINT, // ? it's not Print_Screen, so what is it?
    INPUT_EXECUTE, // ?
    INPUT_PRINT_SCREEN,
    INPUT_INSERT,
    INPUT_DELETE,
    INPUT_HELP, // ?

    INPUT_1, INPUT_2, INPUT_3, INPUT_4, INPUT_5, INPUT_6, INPUT_7, INPUT_8, INPUT_9, INPUT_0,

    INPUT_A, INPUT_B, INPUT_C, INPUT_D, INPUT_E, INPUT_F, INPUT_G, INPUT_H, INPUT_I, INPUT_J, INPUT_K, INPUT_L, INPUT_M,
    INPUT_N, INPUT_O, INPUT_P, INPUT_Q, INPUT_R, INPUT_S, INPUT_T, INPUT_U, INPUT_V, INPUT_W, INPUT_X, INPUT_Y, INPUT_Z,

    INPUT_LEFT_WINDOWS,
    INPUT_RIGHT_WINDOWS,
    INPUT_APPS, // ?

    INPUT_SLEEP,

    INPUT_NP_0, INPUT_NP_1, INPUT_NP_2, INPUT_NP_3, INPUT_NP_4, INPUT_NP_5, INPUT_NP_6, INPUT_NP_7, INPUT_NP_8, INPUT_NP_9,

    INPUT_MULTIPLY,
    INPUT_ADD,
    INPUT_SEPARATOR, // comma?
    INPUT_SUBTRACT,
    INPUT_DECIMAL, // period?
    INPUT_DIVIDE, // forward_slash?

    INPUT_F1, INPUT_F2, INPUT_F3, INPUT_F4, INPUT_F5, INPUT_F6, INPUT_F7, INPUT_F8, INPUT_F9, INPUT_F10, INPUT_F11, INPUT_F12,

    INPUT_NUM_LOCK,
    INPUT_SCROLL_LOCK,

    INPUT_SEMICOLON,
    INPUT_PLUS,
    INPUT_COMMA,
    INPUT_MINUS,
    INPUT_PERIOD,
    INPUT_FORWARD_SLASH,
    INPUT_TILDE,
    INPUT_LEFT_SQUARE,
    INPUT_BACK_SLASH,
    INPUT_RIGHT_SQUARE,
    INPUT_APOSTROPHE,

// todo(josh): check these out
// #define VK_OEM_1          0xBA
// #define VK_OEM_PLUS       0xBB
// #define VK_OEM_COMMA      0xBC
// #define VK_OEM_MINUS      0xBD
// #define VK_OEM_PERIOD     0xBE
// #define VK_OEM_2          0xBF
// #define VK_OEM_3          0xC0
// #define VK_OEM_4          0xDB
// #define VK_OEM_5          0xDC
// #define VK_OEM_6          0xDD
// #define VK_OEM_7          0xDE
// #define VK_OEM_8          0xDF

// todo(josh): gamepad
// #define VK_GAMEPAD_A                         0xC3
// #define VK_GAMEPAD_B                         0xC4
// #define VK_GAMEPAD_X                         0xC5
// #define VK_GAMEPAD_Y                         0xC6
// #define VK_GAMEPAD_RIGHT_SHOULDER            0xC7
// #define VK_GAMEPAD_LEFT_SHOULDER             0xC8
// #define VK_GAMEPAD_LEFT_TRIGGER              0xC9
// #define VK_GAMEPAD_RIGHT_TRIGGER             0xCA
// #define VK_GAMEPAD_DPAD_UP                   0xCB
// #define VK_GAMEPAD_DPAD_DOWN                 0xCC
// #define VK_GAMEPAD_DPAD_LEFT                 0xCD
// #define VK_GAMEPAD_DPAD_RIGHT                0xCE
// #define VK_GAMEPAD_MENU                      0xCF
// #define VK_GAMEPAD_VIEW                      0xD0
// #define VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON    0xD1
// #define VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON   0xD2
// #define VK_GAMEPAD_LEFT_THUMBSTICK_UP        0xD3
// #define VK_GAMEPAD_LEFT_THUMBSTICK_DOWN      0xD4
// #define VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT     0xD5
// #define VK_GAMEPAD_LEFT_THUMBSTICK_LEFT      0xD6
// #define VK_GAMEPAD_RIGHT_THUMBSTICK_UP       0xD7
// #define VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN     0xD8
// #define VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT    0xD9
// #define VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT     0xDA

    INPUT_COUNT,
};

struct Window {
    HWND handle;
    HDC windows_device_context;
    bool should_close;
    bool is_focused;

    int width;
    int height;
    float aspect;
    Vector2 size;

    Vector2 mouse_position_pixel;
    Vector2 mouse_position_unit;
    Vector2 mouse_position_pixel_delta;

    float mouse_scroll;

    int mouse_capture_sum;
    bool updated_at_least_once;

    bool inputs_held[INPUT_COUNT];
    bool inputs_down[INPUT_COUNT];
    bool inputs_up  [INPUT_COUNT];
};

double time_now();
Window create_window(int width, int height);
void update_window(Window *window);

bool get_input(Window *window, Input input);
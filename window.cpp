#include "window.h"

Input windows_key_mapping[256];

void init_platform() {
    windows_key_mapping[0x01] = INPUT_MOUSE_LEFT;
    windows_key_mapping[0x02] = INPUT_MOUSE_RIGHT;
    windows_key_mapping[0x04] = INPUT_MOUSE_MIDDLE;

    windows_key_mapping[0x08] = INPUT_BACKSPACE;
    windows_key_mapping[0x09] = INPUT_TAB;

    windows_key_mapping[0x0C] = INPUT_CLEAR;
    windows_key_mapping[0x0D] = INPUT_ENTER;

    windows_key_mapping[0x10] = INPUT_SHIFT;
    windows_key_mapping[0x11] = INPUT_CONTROL;
    windows_key_mapping[0x12] = INPUT_ALT;
    windows_key_mapping[0x13] = INPUT_PAUSE;
    windows_key_mapping[0x14] = INPUT_CAPS_LOCK;

    windows_key_mapping[0x1B] = INPUT_ESCAPE;

    windows_key_mapping[0x20] = INPUT_SPACE;
    windows_key_mapping[0x21] = INPUT_PAGE_UP;
    windows_key_mapping[0x22] = INPUT_PAGE_DOWN;
    windows_key_mapping[0x23] = INPUT_END;
    windows_key_mapping[0x24] = INPUT_HOME;
    windows_key_mapping[0x25] = INPUT_LEFT;
    windows_key_mapping[0x26] = INPUT_UP;
    windows_key_mapping[0x27] = INPUT_RIGHT;
    windows_key_mapping[0x28] = INPUT_DOWN;
    windows_key_mapping[0x29] = INPUT_SELECT;
    windows_key_mapping[0x2A] = INPUT_PRINT;
    windows_key_mapping[0x2B] = INPUT_EXECUTE;
    windows_key_mapping[0x2C] = INPUT_PRINT_SCREEN;
    windows_key_mapping[0x2D] = INPUT_INSERT;
    windows_key_mapping[0x2E] = INPUT_DELETE;
    windows_key_mapping[0x2F] = INPUT_HELP;

    windows_key_mapping['1'] = INPUT_1;
    windows_key_mapping['2'] = INPUT_2;
    windows_key_mapping['3'] = INPUT_3;
    windows_key_mapping['4'] = INPUT_4;
    windows_key_mapping['5'] = INPUT_5;
    windows_key_mapping['6'] = INPUT_6;
    windows_key_mapping['7'] = INPUT_7;
    windows_key_mapping['8'] = INPUT_8;
    windows_key_mapping['9'] = INPUT_9;
    windows_key_mapping['0'] = INPUT_0;

    windows_key_mapping['A'] = INPUT_A;
    windows_key_mapping['B'] = INPUT_B;
    windows_key_mapping['C'] = INPUT_C;
    windows_key_mapping['D'] = INPUT_D;
    windows_key_mapping['E'] = INPUT_E;
    windows_key_mapping['F'] = INPUT_F;
    windows_key_mapping['G'] = INPUT_G;
    windows_key_mapping['H'] = INPUT_H;
    windows_key_mapping['I'] = INPUT_I;
    windows_key_mapping['J'] = INPUT_J;
    windows_key_mapping['K'] = INPUT_K;
    windows_key_mapping['L'] = INPUT_L;
    windows_key_mapping['M'] = INPUT_M;
    windows_key_mapping['N'] = INPUT_N;
    windows_key_mapping['O'] = INPUT_O;
    windows_key_mapping['P'] = INPUT_P;
    windows_key_mapping['Q'] = INPUT_Q;
    windows_key_mapping['R'] = INPUT_R;
    windows_key_mapping['S'] = INPUT_S;
    windows_key_mapping['T'] = INPUT_T;
    windows_key_mapping['U'] = INPUT_U;
    windows_key_mapping['V'] = INPUT_V;
    windows_key_mapping['W'] = INPUT_W;
    windows_key_mapping['X'] = INPUT_X;
    windows_key_mapping['Y'] = INPUT_Y;
    windows_key_mapping['Z'] = INPUT_Z;

    windows_key_mapping[0x5B] = INPUT_LEFT_WINDOWS;
    windows_key_mapping[0x5C] = INPUT_RIGHT_WINDOWS;
    windows_key_mapping[0x5D] = INPUT_APPS;

    windows_key_mapping[0x5F] = INPUT_SLEEP;

    windows_key_mapping[0x60] = INPUT_NP_0;
    windows_key_mapping[0x61] = INPUT_NP_1;
    windows_key_mapping[0x62] = INPUT_NP_2;
    windows_key_mapping[0x63] = INPUT_NP_3;
    windows_key_mapping[0x64] = INPUT_NP_4;
    windows_key_mapping[0x65] = INPUT_NP_5;
    windows_key_mapping[0x66] = INPUT_NP_6;
    windows_key_mapping[0x67] = INPUT_NP_7;
    windows_key_mapping[0x68] = INPUT_NP_8;
    windows_key_mapping[0x69] = INPUT_NP_9;
    windows_key_mapping[0x6A] = INPUT_MULTIPLY;
    windows_key_mapping[0x6B] = INPUT_ADD;
    windows_key_mapping[0x6C] = INPUT_SEPARATOR;
    windows_key_mapping[0x6D] = INPUT_SUBTRACT;
    windows_key_mapping[0x6E] = INPUT_DECIMAL;
    windows_key_mapping[0x6F] = INPUT_DIVIDE;
    windows_key_mapping[0x70] = INPUT_F1;
    windows_key_mapping[0x71] = INPUT_F2;
    windows_key_mapping[0x72] = INPUT_F3;
    windows_key_mapping[0x73] = INPUT_F4;
    windows_key_mapping[0x74] = INPUT_F5;
    windows_key_mapping[0x75] = INPUT_F6;
    windows_key_mapping[0x76] = INPUT_F7;
    windows_key_mapping[0x77] = INPUT_F8;
    windows_key_mapping[0x78] = INPUT_F9;
    windows_key_mapping[0x79] = INPUT_F10;
    windows_key_mapping[0x7A] = INPUT_F11;
    windows_key_mapping[0x7B] = INPUT_F12;

    windows_key_mapping[0x90] = INPUT_NUM_LOCK;
    windows_key_mapping[0x91] = INPUT_SCROLL_LOCK;

    windows_key_mapping[0xBA] = INPUT_SEMICOLON;
    windows_key_mapping[0xBB] = INPUT_PLUS;
    windows_key_mapping[0xBC] = INPUT_COMMA;
    windows_key_mapping[0xBD] = INPUT_MINUS;
    windows_key_mapping[0xBE] = INPUT_PERIOD;
    windows_key_mapping[0xBF] = INPUT_FORWARD_SLASH;
    windows_key_mapping[0xC0] = INPUT_TILDE;
    windows_key_mapping[0xDB] = INPUT_LEFT_SQUARE;
    windows_key_mapping[0xDC] = INPUT_BACK_SLASH;
    windows_key_mapping[0xDD] = INPUT_RIGHT_SQUARE;
    windows_key_mapping[0xDE] = INPUT_APOSTROPHE;

    // todo(josh)
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
}

double time_now() {
    FILETIME file_time = {};
    GetSystemTimeAsFileTime(&file_time);
    i64 t = (i64)(((u64)file_time.dwLowDateTime) | ((u64)file_time.dwHighDateTime) << 32);
    return (double)(t - 0x019db1ded53e8000) / 10000000.0;
}

Window *currently_processing_window;

LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
    switch (msg) {
        case WM_SIZE: {
            assert(currently_processing_window != nullptr);

            // todo(josh): figure out what to do with wparam
            switch (w) {
                case 0 /* SIZE_RESTORED  */: break;
                case 1 /* SIZE_MINIMIZED */: break;
                case 2 /* SIZE_MAXIMIZED */: break;
                case 3 /* SIZE_MAXSHOW   */: break;
                case 4 /* SIZE_MAXHIDE   */: break;
            }

            u32 width  = LOWORD(l);
            u32 height = HIWORD(l);

            if (width  <= 0) width  = 1;
            if (height <= 0) height = 1;

            printf("New window size: %dx%d\n", width, height);
            currently_processing_window->width  = width;
            currently_processing_window->height = height;
            currently_processing_window->aspect = width / height;
            currently_processing_window->size   = v2(currently_processing_window->width, currently_processing_window->height);
            return 0;
        }
        case WM_MOUSEMOVE: {
            u32 x = LOWORD(l);
            u32 y = HIWORD(l);
            Vector2 old_pos = currently_processing_window->mouse_position_pixel;
            currently_processing_window->mouse_position_pixel = v2((f32)x, currently_processing_window->size.y - (f32)y);
            currently_processing_window->mouse_position_unit  = currently_processing_window->mouse_position_pixel / currently_processing_window->size;
            if (currently_processing_window->updated_at_least_once) {
                currently_processing_window->mouse_position_pixel_delta = currently_processing_window->mouse_position_pixel - old_pos;
            }
            return 0;
        }
        case WM_KEYDOWN: {
            assert(currently_processing_window != nullptr);
            Input input = windows_key_mapping[w];
            // if (!currently_processing_window->inputs_held[input]) {
            //     currently_processing_window->inputs_down[input] = true;
            // }
            currently_processing_window->inputs_held[input] = true;
            return 0;
        }
        case WM_SYSKEYDOWN: {
            assert(currently_processing_window != nullptr);
            Input input = windows_key_mapping[w];
            // if (!currently_processing_window->inputs_held[input]) {
            //     currently_processing_window->inputs_down[input] = true;
            // }
            currently_processing_window->inputs_held[input] = true;
            return 0;
        }
        case WM_KEYUP: {
            Input input = windows_key_mapping[w];
            // g_inputs->inputs_up[input] = true;
            currently_processing_window->inputs_held[input] = false;
            return 0;
        }
        case WM_SYSKEYUP: {
            Input input = windows_key_mapping[w];
            // g_inputs->inputs_up[input] = true;
            currently_processing_window->inputs_held[input] = false;
            return 0;
        }
        /*
        case WM_CHAR: {
            when DEVELOPER {
                imgui.io_add_input_character(imgui.get_io(), u32(wparam));
            }
        }
        case WM_MOUSEWHEEL: {
            scroll := transmute(i16)HIWORD_W(wparam) / 120; // note(josh): 120 is WHEEL_DELTA in windows
            currently_processing_window.mouse_scroll = cast(f32)scroll;
        }
        case WM_LBUTTONDOWN: {
            if mouse_capture_sum == 0 do set_capture(currently_processing_window.platform_data.window_handle);
            mouse_capture_sum += 1;

            if !g_inputs.inputs_held[.Mouse_Left] {
                g_inputs.inputs_down[.Mouse_Left] = true;
            }
            g_inputs.inputs_held[.Mouse_Left] = true;
        }
        case WM_LBUTTONUP: {
            mouse_capture_sum -= 1;
            if mouse_capture_sum == 0 do release_capture();

            g_inputs.inputs_up[.Mouse_Left]   = true;
            g_inputs.inputs_held[.Mouse_Left] = false;
        }
        case WM_MBUTTONDOWN: {
            if mouse_capture_sum == 0 do set_capture(currently_processing_window.platform_data.window_handle);
            mouse_capture_sum += 1;

            if !g_inputs.inputs_held[.Mouse_Middle] {
                g_inputs.inputs_down[.Mouse_Middle] = true;
            }
            g_inputs.inputs_held[.Mouse_Middle] = true;
        }
        case WM_MBUTTONUP: {
            mouse_capture_sum -= 1;
            if mouse_capture_sum == 0 do release_capture();

            g_inputs.inputs_up[.Mouse_Middle]   = true;
            g_inputs.inputs_held[.Mouse_Middle] = false;
        }
        case WM_RBUTTONDOWN: {
            if mouse_capture_sum == 0 do set_capture(currently_processing_window.platform_data.window_handle);
            mouse_capture_sum += 1;

            if !g_inputs.inputs_held[.Mouse_Right] {
                g_inputs.inputs_down[.Mouse_Right] = true;
            }
            g_inputs.inputs_held[.Mouse_Right] = true;
        }
        case WM_RBUTTONUP: {
            mouse_capture_sum -= 1;
            if mouse_capture_sum == 0 do release_capture();

            g_inputs.inputs_up[.Mouse_Right]   = true;
            g_inputs.inputs_held[.Mouse_Right] = false;
        }
        case WM_ACTIVATEAPP: {
            currently_processing_window.is_focused = cast(bool)wparam;
        }
        case WM_CLOSE: {
            currently_processing_window.should_close = true;
        }
        */
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
        default: {
            // printf("Unhandled windows message: %u\n", msg);
        }
    }

    return DefWindowProc(hwnd, msg, w, l);
}

Window create_window(int width, int height) {
    const char *CLASS_NAME = "my window class";
    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_OWNDC;
    wc.hCursor = LoadCursorA(nullptr, IDC_ARROW);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = CLASS_NAME;
    auto c = RegisterClassExA(&wc);
    assert(c != 0);

    Window window = {};
    window.width = width;
    window.height = height;
    currently_processing_window = &window;
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
    currently_processing_window = nullptr;

    assert(window.handle != nullptr && "window.handle was null");
    window.windows_device_context = GetDC(window.handle);
    return window;
}

void update_window(Window *window) {
    assert(currently_processing_window == nullptr);
    currently_processing_window = window;
    currently_processing_window->mouse_position_pixel_delta = {};
    MSG msg = {};
    while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    currently_processing_window->updated_at_least_once = true;
    currently_processing_window = nullptr;
}



bool get_input(Window *window, Input input) {
    return window->inputs_held[input];
}
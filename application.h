#pragma once

//
// Platform and graphics backend abstraction.
// #define CFF_APPLICATION_IMPLEMENTATION before including in once place. Include freely wherever else.
//
// Platform:
// #define CFF_PLATFORM_WINDOWS for Windows
//
// Graphics:
// #define CFF_GRAPHICS_DIRECTX11 for DirectX11
//

#ifdef CFF_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "basic.h"
#include "math.h"

#ifdef CFF_GRAPHICS_DIRECTX11
#include <d3d11.h>
#include <d3dcompiler.h>
#endif

#ifdef CFF_APPLICATION_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

// Platform stuff

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

struct Window;

void init_platform();

double time_now();
Window create_window(int width, int height);
void update_window(Window *window);

bool get_input(Window *window, Input input);
bool get_input_down(Window *window, Input input);
bool get_input_up(Window *window, Input input);



#ifdef CFF_PLATFORM_WINDOWS

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

#endif



// Graphics stuff

#ifdef CFF_GRAPHICS_DIRECTX11

typedef ID3D11Buffer *Buffer;

typedef struct {
    ID3D11Texture2D *handle_2d;
    ID3D11Texture2D *handle_msaa_2d;
    ID3D11Texture3D *handle_3d;

    ID3D11ShaderResourceView *shader_resource_view;
    ID3D11UnorderedAccessView *uav;
} Texture_Backend_Data;

typedef struct {
    ID3D11VertexShader *handle;
    ID3D10Blob *blob;
} Vertex_Shader;

typedef ID3D11PixelShader *Pixel_Shader;

typedef ID3D11ComputeShader *Compute_Shader;

typedef ID3D11InputLayout *Vertex_Format;
#endif



enum Texture_Format {
    TF_INVALID,

    TF_R8_UINT,
    TF_R32_INT,
    TF_R32_FLOAT,
    TF_R16G16B16A16_FLOAT,
    TF_R32G32B32A32_FLOAT,
    TF_R8G8B8A8_UINT,
    TF_R8G8B8A8_UINT_SRGB,
    TF_DEPTH_STENCIL,

    TF_COUNT,
};

struct Texture_Format_Info {
    int pixel_size_in_bytes;
    int num_channels;
    bool is_depth_format;
};

enum Texture_Type {
    TT_INVALID,
    TT_2D,
    TT_3D,
    TT_CUBEMAP,

    TT_COUNT,
};

enum Texture_Wrap_Mode {
    TWM_INVALID,

    TWM_LINEAR_WRAP,
    TWM_LINEAR_CLAMP,
    TWM_POINT_WRAP,
    TWM_POINT_CLAMP,

    TWM_COUNT,
};

struct Texture_Description {
    int width;
    int height;
    int depth;
    bool render_target;
    bool cpu_read_target;
    bool uav;
    int sample_count;
    int mipmap_count;
    Texture_Format format;
    Texture_Type type;
    Texture_Wrap_Mode wrap_mode;
    byte *color_data;
};

struct Texture {
    bool valid; // note(josh): just for checking against zero-value
    Texture_Description description;
    Texture_Backend_Data backend;
};

enum Vertex_Field_Type {
    VFT_INVALID,
    VFT_INT,
    VFT_INT2,
    VFT_INT3,
    VFT_INT4,
    VFT_UINT,
    VFT_UINT2,
    VFT_UINT3,
    VFT_UINT4,
    VFT_FLOAT,
    VFT_FLOAT2,
    VFT_FLOAT3,
    VFT_FLOAT4,

    VFT_COUNT,
};

enum Vertex_Field_Step_Type {
    VFST_INVALID,
    VFST_PER_VERTEX,
    VFST_PER_INSTANCE,

    VFST_COUNT,
};

struct Vertex_Field {
    char *semantic;
    char *name;
    int offset;
    Vertex_Field_Type type;
    Vertex_Field_Step_Type step_type;
};

enum Buffer_Type {
    BT_INVALID,
    BT_VERTEX,
    BT_INDEX,
    BT_CONSTANT,

    BT_COUNT,
};

enum Primitive_Topology {
    PT_TRIANGLE_LIST,
    PT_TRIANGLE_STRIP,
    PT_LINE_LIST,
    PT_LINE_STRIP,

    PT_COUNT,
};

#ifndef CFF_MAX_BOUND_TEXTURES
#define CFF_MAX_BOUND_TEXTURES 12
#endif

#ifndef CFF_MAX_BOUND_RENDER_TARGETS
#define CFF_MAX_BOUND_RENDER_TARGETS 8
#endif

#ifndef CFF_MAX_VERTEX_FIELDS
#define CFF_MAX_VERTEX_FIELDS 32 // note(josh): this is arbitrary.
#endif

void init_render_backend(Window *window);

void create_color_and_depth_buffers(Texture_Description description, Texture *out_color_buffer, Texture *out_depth_buffer);
void ensure_texture_size(Texture *texture, int width, int height);

Texture create_texture_from_file(char *filename, Texture_Format format, Texture_Wrap_Mode wrap_mode);
byte *load_texture_data_from_file(char *filename, int *width, int *height);
void delete_texture_data(byte *data);

Texture_Format_Info get_texture_format_info(Texture_Format format);



// the following are all defined by the specific graphics backend we are compiling with.

void init_graphics_driver(Window *window);
void ensure_swap_chain_size(int width, int height);
void get_swapchain_size(int *out_width, int *out_height);

void set_viewport(int x, int y, int width, int height);
void set_depth_test(bool enabled);
void set_backface_cull(bool enabled);
void set_primitive_topology(Primitive_Topology pt);
void set_alpha_blend(bool enabled);

Vertex_Format create_vertex_format(Vertex_Field *fields, int num_fields, Vertex_Shader shader);
void          destroy_vertex_format(Vertex_Format format);
void          bind_vertex_format(Vertex_Format format);

Buffer create_buffer(Buffer_Type type, void *data, int len);
void   update_buffer(Buffer buffer, void *data, int len);
void   destroy_buffer(Buffer buffer);
void   bind_vertex_buffers(Buffer *buffers, int num_buffers, u32 start_slot, u32 *strides, u32 *offsets);
void   bind_index_buffer(Buffer buffer, u32 slot);
void   bind_constant_buffers(Buffer *buffers, int num_buffers, u32 start_slot);

Vertex_Shader  compile_vertex_shader_from_file(wchar_t *filename);
void           destroy_vertex_shader(Vertex_Shader shader);
Pixel_Shader   compile_pixel_shader_from_file(wchar_t *filename);
void           destroy_pixel_shader(Pixel_Shader shader);
void           bind_shaders(Vertex_Shader vertex, Pixel_Shader pixel);
Compute_Shader compile_compute_shader_from_file(wchar_t *filename);
void           bind_compute_shader(Compute_Shader shader);
void           destroy_compute_shader(Compute_Shader shader);

void bind_compute_uav(Texture texture, int slot);
void dispatch_compute(int x, int y, int z);

Texture create_texture(Texture_Description desc);
void    destroy_texture(Texture texture);
void    bind_texture(Texture texture, int slot);
void    unbind_all_textures();
void    copy_texture(Texture dst, Texture src);
void    set_cubemap_textures(Texture texture, byte *face_pixel_data[6]);

void *map_texture(Texture *texture);
void  unmap_texture(Texture *texture);

struct Color_Buffer_Binding {
    Texture texture;
    bool clear;
    Vector4 clear_color;
};

struct Depth_Buffer_Binding {
    Texture texture;
    bool clear;
    float clear_depth;
    // todo(josh): clear_stencil
};

struct Render_Target_Bindings {
    Color_Buffer_Binding color_bindings[CFF_MAX_BOUND_RENDER_TARGETS];
    Depth_Buffer_Binding depth_binding;
};

void set_render_targets(Render_Target_Bindings bindings);
void unset_render_targets();
// void clear_bound_render_targets(Vector4 color);

void issue_draw_call(int vertex_count, int index_count, int instance_count = 0);
void present(bool vsync);






#ifdef CFF_APPLICATION_IMPLEMENTATION

#ifdef CFF_PLATFORM_WINDOWS

static Input windows_key_mapping[256];

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

static Window *currently_processing_window;

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
            if (!currently_processing_window->inputs_held[input]) {
                currently_processing_window->inputs_down[input] = true;
            }
            currently_processing_window->inputs_held[input] = true;
            return 0;
        }
        case WM_SYSKEYDOWN: {
            assert(currently_processing_window != nullptr);
            Input input = windows_key_mapping[w];
            if (!currently_processing_window->inputs_held[input]) {
                currently_processing_window->inputs_down[input] = true;
            }
            currently_processing_window->inputs_held[input] = true;
            return 0;
        }
        case WM_KEYUP: {
            Input input = windows_key_mapping[w];
            currently_processing_window->inputs_up[input] = true;
            currently_processing_window->inputs_held[input] = false;
            return 0;
        }
        case WM_SYSKEYUP: {
            Input input = windows_key_mapping[w];
            currently_processing_window->inputs_up[input] = true;
            currently_processing_window->inputs_held[input] = false;
            return 0;
        }
        case WM_CHAR: {
#ifdef DEVELOPER
            // imgui.io_add_input_character(imgui.get_io(), u32(w));
#endif
            return 0;
        }
        case WM_MOUSEWHEEL: {
            i16 scroll = ((i16)HIWORD(w)) / 120; // note(josh): 120 is WHEEL_DELTA in windows
            currently_processing_window->mouse_scroll = (f32)scroll;
            return 0;
        }
        case WM_LBUTTONDOWN: {
            if (currently_processing_window->mouse_capture_sum == 0) SetCapture(currently_processing_window->handle);
            currently_processing_window->mouse_capture_sum += 1;

            if (!currently_processing_window->inputs_held[INPUT_MOUSE_LEFT]) {
                currently_processing_window->inputs_down[INPUT_MOUSE_LEFT] = true;
            }
            currently_processing_window->inputs_held[INPUT_MOUSE_LEFT] = true;
            return 0;
        }
        case WM_LBUTTONUP: {
            currently_processing_window->mouse_capture_sum -= 1;
            if (currently_processing_window->mouse_capture_sum == 0) ReleaseCapture();

            currently_processing_window->inputs_up[INPUT_MOUSE_LEFT]   = true;
            currently_processing_window->inputs_held[INPUT_MOUSE_LEFT] = false;
            return 0;
        }
        case WM_MBUTTONDOWN: {
            if (currently_processing_window->mouse_capture_sum == 0) SetCapture(currently_processing_window->handle);
            currently_processing_window->mouse_capture_sum += 1;

            if (!currently_processing_window->inputs_held[INPUT_MOUSE_MIDDLE]) {
                currently_processing_window->inputs_down[INPUT_MOUSE_MIDDLE] = true;
            }
            currently_processing_window->inputs_held[INPUT_MOUSE_MIDDLE] = true;
            return 0;
        }
        case WM_MBUTTONUP: {
            currently_processing_window->mouse_capture_sum -= 1;
            if (currently_processing_window->mouse_capture_sum == 0) ReleaseCapture();

            currently_processing_window->inputs_up[INPUT_MOUSE_MIDDLE]   = true;
            currently_processing_window->inputs_held[INPUT_MOUSE_MIDDLE] = false;
            return 0;
        }
        case WM_RBUTTONDOWN: {
            if (currently_processing_window->mouse_capture_sum == 0) SetCapture(currently_processing_window->handle);
            currently_processing_window->mouse_capture_sum += 1;

            if (!currently_processing_window->inputs_held[INPUT_MOUSE_RIGHT]) {
                currently_processing_window->inputs_down[INPUT_MOUSE_RIGHT] = true;
            }
            currently_processing_window->inputs_held[INPUT_MOUSE_RIGHT] = true;
            return 0;
        }
        case WM_RBUTTONUP: {
            currently_processing_window->mouse_capture_sum -= 1;
            if (currently_processing_window->mouse_capture_sum == 0) ReleaseCapture();

            currently_processing_window->inputs_up[INPUT_MOUSE_RIGHT]   = true;
            currently_processing_window->inputs_held[INPUT_MOUSE_RIGHT] = false;
            return 0;
        }
        case WM_ACTIVATEAPP: {
            currently_processing_window->is_focused = (bool)w;
            return 0;
        }
        case WM_CLOSE: {
            currently_processing_window->should_close = true;
            return 0;
        }
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
    currently_processing_window->mouse_scroll = {};
    memset(&currently_processing_window->inputs_down[0], 0, ARRAYSIZE(currently_processing_window->inputs_down));
    memset(&currently_processing_window->inputs_up[0],   0, ARRAYSIZE(currently_processing_window->inputs_up));

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

bool get_input_down(Window *window, Input input) {
    return window->inputs_down[input];
}

bool get_input_up(Window *window, Input input) {
    return window->inputs_up[input];
}

#endif


//
// Common graphics stuff
//

// todo(josh): add a get_texture_format_info(Texture_Format) API
Texture_Format_Info texture_format_infos[TF_COUNT];

#define SWAP_CHAIN_FORMAT      TF_R8G8B8A8_UINT
#define SWAP_CHAIN_FORMAT_SRGB TF_R8G8B8A8_UINT_SRGB

void init_render_backend(Window *window) {
    texture_format_infos[TF_R8_UINT]            = {1,  1, false};
    texture_format_infos[TF_R32_INT]            = {4,  1, false};
    texture_format_infos[TF_R32_FLOAT]          = {4,  1, false};
    texture_format_infos[TF_R16G16B16A16_FLOAT] = {8,  4, false};
    texture_format_infos[TF_R32G32B32A32_FLOAT] = {16, 4, false};
    texture_format_infos[TF_R8G8B8A8_UINT]      = {4,  4, false};
    texture_format_infos[TF_R8G8B8A8_UINT_SRGB] = {4,  4, false};
    texture_format_infos[TF_DEPTH_STENCIL]      = {4,  2, true};

    // make sure all texture format infos are supplied
    for (int i = 0; i < ARRAYSIZE(texture_format_infos); i++) {
        if (texture_format_infos[i].pixel_size_in_bytes == 0) {
            if ((Texture_Format)i != TF_INVALID && (Texture_Format)i != TF_COUNT) {
                printf("Missing texture_format_info for %d\n", i);
                assert(false);
            }
        }
    }

    init_graphics_driver(window);
}

Texture_Format_Info get_texture_format_info(Texture_Format format) {
    return texture_format_infos[format];
}

void create_color_and_depth_buffers(Texture_Description description, Texture *out_color_buffer, Texture *out_depth_buffer) {
    assert(out_color_buffer != nullptr);
    assert(out_depth_buffer != nullptr);

    description.render_target = true;
    *out_color_buffer = create_texture(description);

    description.wrap_mode = TWM_POINT_CLAMP;
    description.format = TF_DEPTH_STENCIL;
    *out_depth_buffer = create_texture(description);
}

void ensure_texture_size(Texture *texture, int width, int height) {
    assert(width != 0);
    assert(height != 0);

    if (texture->description.width != width || texture->description.height != height) {
        printf("Resizing texture %dx%d...\n", width, height);
        Texture_Description desc = texture->description;
        destroy_texture(*texture);
        desc.width  = width;
        desc.height = height;
        *texture = create_texture(desc);
    }
}

Texture create_texture_from_file(char *filename, Texture_Format format, Texture_Wrap_Mode wrap_mode) {
    int width;
    int height;
    byte *color_data = load_texture_data_from_file(filename, &width, &height);
    if (!color_data) {
        printf("create_texture_from_file() couldn't find file: %s\n", filename);
        return {};
    }
    defer(delete_texture_data(color_data));

    Texture_Description texture_description = {};
    texture_description.width = width;
    texture_description.height = height;
    texture_description.color_data = color_data;
    texture_description.format = format;
    texture_description.wrap_mode = wrap_mode;
    texture_description.type = TT_2D;
    Texture texture = create_texture(texture_description);
    return texture;
}

byte *load_texture_data_from_file(char *filename, int *width, int *height) {
    int filedata_len;
    char *filedata = read_entire_file(filename, &filedata_len);
    if (!filedata) {
        printf("load_texture_data_from_file() couldn't find file: %s\n", filename);
        return {};
    }
    assert(filedata != nullptr);
    defer(free(filedata));
    // stbi_set_flip_vertically_on_load(1);
    int n;
    byte *color_data = stbi_load_from_memory((byte *)filedata, filedata_len, width, height, &n, 4);
    assert(color_data);
    return color_data;
}

void delete_texture_data(byte *data) {
    stbi_image_free(data);
}



//
// Specific render backends
//

#ifdef CFF_GRAPHICS_DIRECTX11

struct DirectX {
    ID3D11Device *device;
    ID3D11DeviceContext *device_context;

    IDXGISwapChain *swap_chain_handle;
    int swap_chain_width;
    int swap_chain_height;

    Texture swap_chain_depth_buffer;

    ID3D11RasterizerState *no_cull_rasterizer;
    ID3D11RasterizerState *backface_cull_rasterizer;
    ID3D11DepthStencilState *depth_test_state;
    ID3D11DepthStencilState *no_depth_test_state;
    ID3D11SamplerState *linear_wrap_sampler;
    ID3D11SamplerState *linear_clamp_sampler;
    ID3D11SamplerState *point_wrap_sampler;
    ID3D11SamplerState *point_clamp_sampler;
    ID3D11BlendState *alpha_blend_state;
    ID3D11BlendState *no_alpha_blend_state;

    ID3D11RenderTargetView   *cur_rtvs[CFF_MAX_BOUND_RENDER_TARGETS];
    Texture current_render_targets[CFF_MAX_BOUND_RENDER_TARGETS]; // note(josh): for resolving MSAA
    ID3D11DepthStencilView   *cur_dsv;
    ID3D11ShaderResourceView *cur_srvs[CFF_MAX_BOUND_TEXTURES];
    ID3D11UnorderedAccessView *cur_uavs[8]; // todo(josh): figure out a good constant
};

static DirectX directx;

#include "dearimgui.cpp"

static DXGI_FORMAT dx_texture_format_mapping[TF_COUNT];

ID3D11RenderTargetView *dx_create_render_target_view(ID3D11Texture2D *backing_texture, Texture_Format format, bool msaa) {
    ASSERT(format != TF_INVALID);
    D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc = {};
    render_target_view_desc.Format        = dx_texture_format_mapping[format];
    if (msaa) {
        render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
    }
    else {
        render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    }
    ID3D11RenderTargetView *render_target_view = {};
    auto result = directx.device->CreateRenderTargetView(backing_texture, &render_target_view_desc, &render_target_view);
    ASSERT(result == S_OK);
    return render_target_view;
}

ID3D11DepthStencilView *dx_create_depth_stencil_view(ID3D11Texture2D *backing_texture, Texture_Format format, bool msaa) {
    ASSERT(format != TF_INVALID);
    ASSERT(texture_format_infos[format].is_depth_format);
    D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc = {};
    depth_stencil_view_desc.Format = dx_texture_format_mapping[format];
    if (msaa) {
        depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
    }
    else {
        depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    }
    ID3D11DepthStencilView *depth_stencil_view = {};
    auto result = directx.device->CreateDepthStencilView((ID3D11Resource *)backing_texture, nullptr, &depth_stencil_view);
    ASSERT(result == S_OK);
    return depth_stencil_view;
}

#define SWAP_CHAIN_BUFFER_COUNT 2

void init_graphics_driver(Window *window) {
    dx_texture_format_mapping[TF_R8_UINT]            = DXGI_FORMAT_R8_UNORM;
    dx_texture_format_mapping[TF_R32_INT]            = DXGI_FORMAT_R32_SINT;
    dx_texture_format_mapping[TF_R32_FLOAT]          = DXGI_FORMAT_R32_FLOAT;
    dx_texture_format_mapping[TF_R16G16B16A16_FLOAT] = DXGI_FORMAT_R16G16B16A16_FLOAT;
    dx_texture_format_mapping[TF_R32G32B32A32_FLOAT] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    dx_texture_format_mapping[TF_R8G8B8A8_UINT]      = DXGI_FORMAT_R8G8B8A8_UNORM;
    dx_texture_format_mapping[TF_R8G8B8A8_UINT_SRGB] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    dx_texture_format_mapping[TF_DEPTH_STENCIL]      = DXGI_FORMAT_D24_UNORM_S8_UINT;

    // make sure all texture formats have a mapping
    for (int i = 0; i < ARRAYSIZE(dx_texture_format_mapping); i++) {
        if (dx_texture_format_mapping[i] == 0) {
            if ((Texture_Format)i != TF_INVALID && (Texture_Format)i != TF_COUNT) {
                printf("Missing dx texture format mapping for %d\n", i);
                ASSERT(false);
            }
        }
    }

    // Create swap chain
    DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
    swap_chain_desc.BufferCount                        = SWAP_CHAIN_BUFFER_COUNT;
    swap_chain_desc.SwapEffect                         = DXGI_SWAP_EFFECT_FLIP_DISCARD; // todo(josh): use DXGI_SWAP_EFFECT_DISCARD (or something else) on non-Windows 10
    swap_chain_desc.BufferDesc.Width                   = (u32)window->width;
    swap_chain_desc.BufferDesc.Height                  = (u32)window->height;
    swap_chain_desc.BufferDesc.Format                  = dx_texture_format_mapping[SWAP_CHAIN_FORMAT];
    swap_chain_desc.BufferDesc.RefreshRate.Numerator   = 60; // todo(josh): query monitor refresh rate.
    swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
    swap_chain_desc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.OutputWindow                       = window->handle;
    swap_chain_desc.SampleDesc.Count                   = 1;
    swap_chain_desc.SampleDesc.Quality                 = 0;
    swap_chain_desc.Windowed                           = true;

    directx.swap_chain_width  = window->width;
    directx.swap_chain_height = window->height;

    D3D_FEATURE_LEVEL requested_feature_level = D3D_FEATURE_LEVEL_11_0;
    D3D_FEATURE_LEVEL actual_feature_level = {};
    auto result = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_DEBUG,
        &requested_feature_level,
        1,
        D3D11_SDK_VERSION, // jake pls
        &swap_chain_desc,
        &directx.swap_chain_handle,
        &directx.device,
        &actual_feature_level,
        &directx.device_context);

    // todo(josh): if the hardware device fails, try making a WARP device
    ASSERT(result == S_OK);

    Texture_Description depth_texture_desc = {};
    depth_texture_desc.type = TT_2D;
    depth_texture_desc.format = TF_DEPTH_STENCIL;
    depth_texture_desc.wrap_mode = TWM_POINT_CLAMP;
    depth_texture_desc.width = window->width;
    depth_texture_desc.height = window->height;
    depth_texture_desc.render_target = true;
    directx.swap_chain_depth_buffer = create_texture(depth_texture_desc);

    // Make no cull rasterizer
    D3D11_RASTERIZER_DESC no_cull_rasterizer_desc = {};
    no_cull_rasterizer_desc.FillMode = D3D11_FILL_SOLID;
    no_cull_rasterizer_desc.CullMode = D3D11_CULL_NONE;
    no_cull_rasterizer_desc.DepthClipEnable = false;
    no_cull_rasterizer_desc.MultisampleEnable = true; // todo(josh): can I just have multisample enabled on all rasterizers?
    result = directx.device->CreateRasterizerState(&no_cull_rasterizer_desc, &directx.no_cull_rasterizer);
    ASSERT(result == S_OK);

    // Make backface cull rasterizer
    D3D11_RASTERIZER_DESC backface_cull_rasterizer_desc = {};
    backface_cull_rasterizer_desc.FillMode = D3D11_FILL_SOLID;
    backface_cull_rasterizer_desc.CullMode = D3D11_CULL_BACK;
    backface_cull_rasterizer_desc.DepthClipEnable = true;
    backface_cull_rasterizer_desc.MultisampleEnable = true; // todo(josh): can I just have multisample enabled on all rasterizers?
    result = directx.device->CreateRasterizerState(&backface_cull_rasterizer_desc, &directx.backface_cull_rasterizer);
    ASSERT(result == S_OK);

    // Depth test state
    D3D11_DEPTH_STENCIL_DESC depth_test_stencil_desc = {};
    depth_test_stencil_desc.DepthEnable                  = true;
    depth_test_stencil_desc.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK_ALL;
    depth_test_stencil_desc.DepthFunc                    = D3D11_COMPARISON_LESS_EQUAL;
    depth_test_stencil_desc.StencilEnable                = true;
    depth_test_stencil_desc.StencilReadMask              = 0xff;
    depth_test_stencil_desc.StencilWriteMask             = 0xff;
    depth_test_stencil_desc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
    depth_test_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    depth_test_stencil_desc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
    depth_test_stencil_desc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
    depth_test_stencil_desc.BackFace.StencilFunc         = D3D11_COMPARISON_ALWAYS;
    depth_test_stencil_desc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
    depth_test_stencil_desc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_KEEP;
    depth_test_stencil_desc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
    result = directx.device->CreateDepthStencilState(&depth_test_stencil_desc, &directx.depth_test_state);
    ASSERT(result == S_OK);

    // No depth test state
    // todo(josh): should we disable stencil here?
    D3D11_DEPTH_STENCIL_DESC no_depth_test_stencil_desc = depth_test_stencil_desc;
    no_depth_test_stencil_desc.DepthEnable = false;
    no_depth_test_stencil_desc.DepthFunc   = D3D11_COMPARISON_ALWAYS;
    result = directx.device->CreateDepthStencilState(&no_depth_test_stencil_desc, &directx.no_depth_test_state);
    ASSERT(result == S_OK);

    // linear wrap sampler
    D3D11_SAMPLER_DESC linear_wrap_sampler_desc = {};
    linear_wrap_sampler_desc.Filter   = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    linear_wrap_sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    linear_wrap_sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    linear_wrap_sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    linear_wrap_sampler_desc.MinLOD = -FLT_MAX;
    linear_wrap_sampler_desc.MaxLOD = FLT_MAX;
    result = directx.device->CreateSamplerState(&linear_wrap_sampler_desc, &directx.linear_wrap_sampler);
    ASSERT(result == S_OK);

    // linear clamp sampler
    D3D11_SAMPLER_DESC linear_clamp_sampler_desc = {};
    linear_clamp_sampler_desc.Filter   = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    linear_clamp_sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    linear_clamp_sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    linear_clamp_sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    linear_clamp_sampler_desc.MinLOD = -FLT_MAX;
    linear_clamp_sampler_desc.MaxLOD = FLT_MAX;
    result = directx.device->CreateSamplerState(&linear_clamp_sampler_desc, &directx.linear_clamp_sampler);
    ASSERT(result == S_OK);

    // point wrap sampler
    D3D11_SAMPLER_DESC point_wrap_sampler_desc = {};
    point_wrap_sampler_desc.Filter   = D3D11_FILTER_MIN_MAG_MIP_POINT;
    point_wrap_sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    point_wrap_sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    point_wrap_sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    point_wrap_sampler_desc.MinLOD = -FLT_MAX;
    point_wrap_sampler_desc.MaxLOD = FLT_MAX;
    result = directx.device->CreateSamplerState(&point_wrap_sampler_desc, &directx.point_wrap_sampler);
    ASSERT(result == S_OK);

    // point clamp sampler
    D3D11_SAMPLER_DESC point_clamp_sampler_desc = {};
    point_clamp_sampler_desc.Filter   = D3D11_FILTER_MIN_MAG_MIP_POINT;
    point_clamp_sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    point_clamp_sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    point_clamp_sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    point_clamp_sampler_desc.MinLOD = -FLT_MAX;
    point_clamp_sampler_desc.MaxLOD = FLT_MAX;
    result = directx.device->CreateSamplerState(&point_clamp_sampler_desc, &directx.point_clamp_sampler);
    ASSERT(result == S_OK);

    // alpha blend state
    D3D11_BLEND_DESC alpha_blend_desc = {};
    alpha_blend_desc.RenderTarget[0].BlendEnable    = true;
    alpha_blend_desc.RenderTarget[0].SrcBlend       = D3D11_BLEND_SRC_ALPHA;
    alpha_blend_desc.RenderTarget[0].DestBlend      = D3D11_BLEND_INV_SRC_ALPHA;
    alpha_blend_desc.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
    alpha_blend_desc.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_SRC_ALPHA;
    alpha_blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    alpha_blend_desc.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;
    alpha_blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    result = directx.device->CreateBlendState(&alpha_blend_desc, &directx.alpha_blend_state);
    ASSERT(result == S_OK);

    // no alpha blend state
    D3D11_BLEND_DESC no_blend_desc = {};
    no_blend_desc.RenderTarget[0].BlendEnable    = false;
    no_blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    result = directx.device->CreateBlendState(&no_blend_desc, &directx.no_alpha_blend_state);
    ASSERT(result == S_OK);

    // init_dear_imgui(window, directx.device, directx.device_context);
    init_dear_imgui();
}

void set_viewport(int x, int y, int width, int height) {
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = width;
    viewport.Height = height;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1;
    directx.device_context->RSSetViewports(1, &viewport);
}

void set_depth_test(bool enabled) {
    if (enabled) {
        directx.device_context->OMSetDepthStencilState(directx.depth_test_state, 0);
    }
    else {
        directx.device_context->OMSetDepthStencilState(directx.no_depth_test_state, 0);
    }
}

void set_backface_cull(bool enabled) {
    if (enabled) {
        directx.device_context->RSSetState(directx.backface_cull_rasterizer);
    }
    else {
        directx.device_context->RSSetState(directx.no_cull_rasterizer);
    }
}

void set_primitive_topology(Primitive_Topology pt) {
    switch (pt) {
        case PT_TRIANGLE_LIST:  directx.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);  break;
        case PT_TRIANGLE_STRIP: directx.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP); break;
        case PT_LINE_LIST:      directx.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);      break;
        case PT_LINE_STRIP:     directx.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);     break;
        default: {
            ASSERTF(false, "Unknown Primitive_Topology: %d\n", pt);
        }
    }
}

void set_alpha_blend(bool enabled) {
    float blend_factor[4] = {1, 1, 1, 1};
    if (enabled) {
        directx.device_context->OMSetBlendState(directx.alpha_blend_state, blend_factor, 0xffffffff);
    }
    else {
        directx.device_context->OMSetBlendState(directx.no_alpha_blend_state, blend_factor, 0xffffffff);
    }
}

void ensure_swap_chain_size(int width, int height) {
    ASSERT(width != 0);
    ASSERT(height != 0);

    if (directx.swap_chain_width != width || directx.swap_chain_height != height) {
        printf("Resizing swap chain %dx%d...\n", width, height);

        auto result = directx.swap_chain_handle->ResizeBuffers(SWAP_CHAIN_BUFFER_COUNT, (u32)width, (u32)height, dx_texture_format_mapping[SWAP_CHAIN_FORMAT], 0);
        ASSERT(result == S_OK);
        directx.swap_chain_width  = width;
        directx.swap_chain_height = height;

        ensure_texture_size(&directx.swap_chain_depth_buffer, width, height);
    }
}

void get_swapchain_size(int *out_width, int *out_height) {
    *out_width  = directx.swap_chain_width;
    *out_height = directx.swap_chain_height;
}

DXGI_FORMAT dx_vertex_field_type(Vertex_Field_Type vft) {
    switch (vft) {
        case VFT_INT:    return DXGI_FORMAT_R32_SINT;
        case VFT_INT2:   return DXGI_FORMAT_R32G32_SINT;
        case VFT_INT3:   return DXGI_FORMAT_R32G32B32_SINT;
        case VFT_INT4:   return DXGI_FORMAT_R32G32B32A32_SINT;
        case VFT_UINT:   return DXGI_FORMAT_R32_UINT;
        case VFT_UINT2:  return DXGI_FORMAT_R32G32_UINT;
        case VFT_UINT3:  return DXGI_FORMAT_R32G32B32_UINT;
        case VFT_UINT4:  return DXGI_FORMAT_R32G32B32A32_UINT;
        case VFT_FLOAT:  return DXGI_FORMAT_R32_FLOAT;
        case VFT_FLOAT2: return DXGI_FORMAT_R32G32_FLOAT;
        case VFT_FLOAT3: return DXGI_FORMAT_R32G32B32_FLOAT;
        case VFT_FLOAT4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
        default: {
            ASSERTF(false, "Unknown vertex format type: %d", vft);
            return DXGI_FORMAT_UNKNOWN;
        }
    }
}

Vertex_Format create_vertex_format(Vertex_Field *fields, int num_fields, Vertex_Shader shader) {
    ASSERTF(num_fields <= CFF_MAX_VERTEX_FIELDS, "Too many vertex fields: %d vs %d. You can override the max vertex fields by defining CFF_MAX_VERTEX_FIELDS before including render_backend.h", num_fields, CFF_MAX_VERTEX_FIELDS);
    D3D11_INPUT_ELEMENT_DESC input_elements[CFF_MAX_VERTEX_FIELDS] = {};
    for (int idx = 0; idx < num_fields; idx++) {
        D3D11_INPUT_ELEMENT_DESC *desc = &input_elements[idx];
        Vertex_Field *field = &fields[idx];

        D3D11_INPUT_CLASSIFICATION step_type = {};
        u32 step_rate = {};
        u32 buffer_slot = {};
        switch (field->step_type) {
            // todo(josh): parameterize step rate
            // todo(josh): parameterize buffer slot
            case VFST_PER_VERTEX:   buffer_slot = 0; step_rate = 0; step_type = D3D11_INPUT_PER_VERTEX_DATA;   break;
            case VFST_PER_INSTANCE: buffer_slot = 1; step_rate = 1; step_type = D3D11_INPUT_PER_INSTANCE_DATA; break;
            default: {
                ASSERTF(false, "Unknown step type: %d\n", field->step_type);
            }
        }

        desc->SemanticName = field->semantic;
        desc->Format = dx_vertex_field_type(field->type);
        desc->InputSlot = buffer_slot;
        desc->AlignedByteOffset = field->offset;
        desc->InputSlotClass = step_type;
        desc->InstanceDataStepRate = step_rate;
    }

    Vertex_Format vertex_format = {};
    auto result = directx.device->CreateInputLayout(input_elements, num_fields, shader.blob->GetBufferPointer(), shader.blob->GetBufferSize(), &vertex_format);
    ASSERTF(result == S_OK, "Failed to create input layout");
    return vertex_format;
}

void bind_vertex_format(Vertex_Format format) {
    directx.device_context->IASetInputLayout(format);
}

Buffer create_buffer(Buffer_Type type, void *data, int len) {
    D3D11_BUFFER_DESC buffer_desc = {};
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.ByteWidth = len;
    switch (type) {
        case BT_VERTEX:   { buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;   break; }
        case BT_INDEX:    { buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;    break; }
        case BT_CONSTANT: { buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; break; }
        default: {
            ASSERTF(false, "Unknown buffer type: %d", type);
        }
    }

    D3D11_SUBRESOURCE_DATA buffer_data = {};
    buffer_data.pSysMem = data;
    Buffer buffer = {};
    auto result = directx.device->CreateBuffer(&buffer_desc, data == nullptr ? nullptr : &buffer_data, &buffer);
    ASSERT(result == S_OK);
    return buffer;
}

void update_buffer(Buffer buffer, void *data, int len) {
    directx.device_context->UpdateSubresource((ID3D11Resource *)buffer, 0, nullptr, data, (u32)len, 0);
}

void bind_vertex_buffers(Buffer *buffers, int num_buffers, u32 start_slot, u32 *strides, u32 *offsets) {
    directx.device_context->IASetVertexBuffers(start_slot, num_buffers, buffers, strides, offsets);
}

void bind_index_buffer(Buffer buffer, u32 slot) {
    // todo(josh): parameterize index type
    directx.device_context->IASetIndexBuffer(buffer, DXGI_FORMAT_R32_UINT, slot);
}

void bind_constant_buffers(Buffer *buffers, int num_buffers, u32 start_slot) {
    directx.device_context->VSSetConstantBuffers(start_slot, num_buffers, buffers);
    directx.device_context->PSSetConstantBuffers(start_slot, num_buffers, buffers);
    directx.device_context->CSSetConstantBuffers(start_slot, num_buffers, buffers);
}

void destroy_buffer(Buffer buffer) {
    buffer->Release();
}

// #define D3D_SHADER_COMPILE_FLAGS (D3DCOMPILE_DEBUG /*| D3DCOMPILE_WARNINGS_ARE_ERRORS*/ | D3DCOMPILE_SKIP_OPTIMIZATION)
#define D3D_SHADER_COMPILE_FLAGS (D3DCOMPILE_DEBUG /*| D3DCOMPILE_WARNINGS_ARE_ERRORS*/)

Vertex_Shader compile_vertex_shader_from_file(wchar_t *filename) { // todo(josh): use a temp allocator to go from char * to wchar_t *
    ID3D10Blob *errors = {};
    ID3D10Blob *vertex_shader_blob = {};
    auto result = D3DCompileFromFile(filename, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", D3D_SHADER_COMPILE_FLAGS, 0, &vertex_shader_blob, &errors);
    if (errors) {
        auto str = (char *)errors->GetBufferPointer();
        printf(str);
        // ASSERT(false);
    }
    ASSERT(result == S_OK);
    ID3D11VertexShader *vertex_shader_handle = {};
    result = directx.device->CreateVertexShader(vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), nullptr, &vertex_shader_handle);
    ASSERT(result == S_OK);
    if (errors) errors->Release();
    Vertex_Shader vertex_shader = {};
    vertex_shader.handle = vertex_shader_handle;
    vertex_shader.blob = vertex_shader_blob;
    return vertex_shader;
}

Pixel_Shader compile_pixel_shader_from_file(wchar_t *filename) { // todo(josh): use a temp allocator to go from char * to wchar_t *
    ID3D10Blob *errors = {};
    ID3D10Blob *pixel_shader_blob = {};
    auto result = D3DCompileFromFile(filename, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", D3D_SHADER_COMPILE_FLAGS, 0, &pixel_shader_blob, &errors);
    if (errors) {
        auto str = (char *)errors->GetBufferPointer();
        printf(str);
        // ASSERT(false);
    }
    ASSERT(result == S_OK);
    ID3D11PixelShader *pixel_shader = {};
    result = directx.device->CreatePixelShader(pixel_shader_blob->GetBufferPointer(), pixel_shader_blob->GetBufferSize(), nullptr, &pixel_shader);
    ASSERT(result == S_OK);
    if (errors) errors->Release();
    pixel_shader_blob->Release();
    return pixel_shader;
}

void bind_shaders(Vertex_Shader vertex, Pixel_Shader pixel) {
    directx.device_context->VSSetShader(vertex.handle, 0, 0);
    directx.device_context->PSSetShader(pixel, 0, 0);
}

Compute_Shader compile_compute_shader_from_file(wchar_t *filename) {
    ID3D10Blob *errors = {};
    ID3D10Blob *compute_blob = {};
    auto result = D3DCompileFromFile(filename, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "cs_5_0", D3D_SHADER_COMPILE_FLAGS, 0, &compute_blob, &errors);
    if (errors) {
        auto str = (char *)errors->GetBufferPointer();
        printf(str);
        // ASSERT(false);
    }
    ASSERT(result == S_OK);
    ID3D11ComputeShader *compute_shader = {};
    result = directx.device->CreateComputeShader(compute_blob->GetBufferPointer(), compute_blob->GetBufferSize(), nullptr, &compute_shader);
    ASSERT(result == S_OK);
    if (errors) errors->Release();
    compute_blob->Release();
    return compute_shader;
}

void bind_compute_shader(Compute_Shader shader) {
    directx.device_context->CSSetShader(shader, nullptr, 0);
}

ID3D11UnorderedAccessView *dx_create_unordered_access_view(Texture texture) {
    ASSERT(texture.description.uav);
    ID3D11UnorderedAccessView * uav = {};
    switch (texture.description.type) {
        case TT_2D: {
            D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
            uav_desc.Format = DXGI_FORMAT_UNKNOWN;
            uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
            auto result = directx.device->CreateUnorderedAccessView(texture.backend.handle_2d, &uav_desc, &uav);
            ASSERT(result == S_OK);
            break;
        }
        case TT_3D: {
            D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
            uav_desc.Format = DXGI_FORMAT_UNKNOWN;
            uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
            uav_desc.Texture3D.WSize = (u32)texture.description.depth;
            auto result = directx.device->CreateUnorderedAccessView(texture.backend.handle_3d, &uav_desc, &uav);
            ASSERT(result == S_OK);
            break;
        }
        case TT_CUBEMAP: {
            ASSERT(false && "unimplemented");
            // D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
            // uav_desc.Format = DXGI_FORMAT_UNKNOWN;
            // uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURECUBE;
            // uav_desc.Texture3D.WSize = (u32)texture.description.depth;
            // auto result = directx.device->CreateUnorderedAccessView(texture.backend.handle_3d, &uav_desc, &uav);
            // ASSERT(result == S_OK);
            break;
        }
        default: {
            ASSERT(false);
        }
    }
    ASSERT(uav != nullptr);
    return uav;
}

void bind_compute_uav(Texture texture, int slot) {
    ASSERT(slot < ARRAYSIZE(directx.cur_uavs));
    if (directx.cur_uavs[slot]) {
        directx.cur_uavs[slot]->Release();
        directx.cur_uavs[slot] = {};
    }

    if (texture.valid) {
        directx.cur_uavs[slot] = dx_create_unordered_access_view(texture);
    }

    directx.device_context->CSSetUnorderedAccessViews(slot, 1, &directx.cur_uavs[slot], nullptr);
}

void dispatch_compute(int x, int y, int z) {
    directx.device_context->Dispatch(x, y, z);
}

void destroy_vertex_shader(Vertex_Shader shader) {
    shader.handle->Release();
    shader.blob->Release();
}

void destroy_pixel_shader(Pixel_Shader shader) {
    shader->Release();
}

void destroy_compute_shader(Compute_Shader shader) {
    shader->Release();
}

void issue_draw_call(int vertex_count, int index_count, int instance_count) {
    if (instance_count == 0) {
        if (index_count > 0) {
            directx.device_context->DrawIndexed((u32)index_count, 0, 0);
        }
        else {
            directx.device_context->Draw((u32)vertex_count, 0);
        }
    }
    else {
        if (index_count > 0) {
            directx.device_context->DrawIndexedInstanced((u32)index_count, (u32)instance_count, 0, 0, 0);
        }
        else {
            directx.device_context->DrawInstanced((u32)vertex_count, (u32)instance_count, 0, 0);
        }
    }
}

Texture create_texture(Texture_Description desc) {
    // todo(josh): check for max texture size?
    ASSERT(desc.width > 0);
    ASSERT(desc.height > 0);

    if (desc.type == TT_INVALID) {
        desc.type = TT_2D;
    }

    if (desc.format == TF_INVALID) {
        desc.format = TF_R8G8B8A8_UINT;
    }

    if (desc.wrap_mode == TWM_INVALID) {
        desc.wrap_mode = TWM_POINT_CLAMP;
    }

    if (desc.sample_count == 0) {
        desc.sample_count = 1;
    }

    if (desc.mipmap_count == 0) {
        desc.mipmap_count = 1;
    }

    ASSERT(desc.mipmap_count <= 1 && "mipmaps are not supported yet");

    DXGI_FORMAT texture_format = dx_texture_format_mapping[desc.format];

    ID3D11Texture2D *texture_handle_2d = {};
    ID3D11Texture3D *texture_handle_3d = {};
    ID3D11Texture2D *msaa_handle_2d = {};
    ID3D11ShaderResourceView *shader_resource_view = {};
    ID3D11UnorderedAccessView *uav = {};
    switch (desc.type) {
        case TT_2D: {
            // Create texture
            D3D11_TEXTURE2D_DESC texture_desc = {};
            texture_desc.Width            = (u32)desc.width;
            texture_desc.Height           = (u32)desc.height;
            texture_desc.MipLevels        = desc.mipmap_count;
            texture_desc.Format           = texture_format;
            texture_desc.SampleDesc.Count = 1;
            texture_desc.ArraySize        = 1;
            if (desc.cpu_read_target) {
                texture_desc.Usage = D3D11_USAGE_STAGING;
                texture_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            }
            else {
                texture_desc.Usage = D3D11_USAGE_DEFAULT;
            }

            if (texture_format_infos[desc.format].is_depth_format) {
                texture_desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
            }
            else {
                if (!desc.cpu_read_target) {
                    texture_desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
                }
                if (desc.uav) {
                    texture_desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
                }
                if (desc.render_target) {
                    texture_desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
                }
            }

            u32 pixel_size = (u32)texture_format_infos[desc.format].pixel_size_in_bytes;
            D3D11_SUBRESOURCE_DATA subresource_data[6] = {
                {desc.color_data, pixel_size * (u32)desc.width,    0},
                {desc.color_data, pixel_size * (u32)desc.width/2,  0},
                {desc.color_data, pixel_size * (u32)desc.width/4,  0},
                {desc.color_data, pixel_size * (u32)desc.width/8,  0},
                {desc.color_data, pixel_size * (u32)desc.width/16, 0},
                {desc.color_data, pixel_size * (u32)desc.width/32, 0},
            };

            auto result = directx.device->CreateTexture2D(&texture_desc, desc.color_data == nullptr ? nullptr : &subresource_data[0], &texture_handle_2d);
            ASSERT(result == S_OK);

            if (desc.sample_count > 1) {
                ASSERT(desc.render_target);
                ASSERT(!desc.uav);
                D3D11_TEXTURE2D_DESC msaa_texture_desc = texture_desc;
                msaa_texture_desc.SampleDesc.Count = desc.sample_count;
                msaa_texture_desc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
                result = directx.device->CreateTexture2D(&msaa_texture_desc, nullptr, &msaa_handle_2d);
                ASSERT(result == S_OK);
            }

            break;
        }
        case TT_3D: {
            // Create texture
            D3D11_TEXTURE3D_DESC texture_desc = {};
            texture_desc.Width     = (u32)desc.width;
            texture_desc.Height    = (u32)desc.height;
            texture_desc.Depth     = (u32)desc.depth;
            texture_desc.MipLevels = desc.mipmap_count;
            texture_desc.Format    = texture_format;
            texture_desc.Usage     = D3D11_USAGE_DEFAULT;

            if (texture_format_infos[desc.format].is_depth_format) {
                texture_desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
            }
            else {
                texture_desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
                if (desc.uav) {
                    texture_desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
                }
                if (desc.render_target) {
                    texture_desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
                }
            }

            u32 pixel_size = (u32)texture_format_infos[desc.format].pixel_size_in_bytes;
            D3D11_SUBRESOURCE_DATA subresource_data[6] = {
                {desc.color_data, pixel_size * (u32)desc.width,    pixel_size * ((u32)desc.width)    * ((u32)desc.height)   },
                {desc.color_data, pixel_size * (u32)desc.width/2,  pixel_size * ((u32)desc.width/2)  * ((u32)desc.height/2) },
                {desc.color_data, pixel_size * (u32)desc.width/4,  pixel_size * ((u32)desc.width/4)  * ((u32)desc.height/4) },
                {desc.color_data, pixel_size * (u32)desc.width/8,  pixel_size * ((u32)desc.width/8)  * ((u32)desc.height/8) },
                {desc.color_data, pixel_size * (u32)desc.width/16, pixel_size * ((u32)desc.width/16) * ((u32)desc.height/16)},
                {desc.color_data, pixel_size * (u32)desc.width/32, pixel_size * ((u32)desc.width/32) * ((u32)desc.height/32)},
            };

            auto result = directx.device->CreateTexture3D(&texture_desc, desc.color_data == nullptr ? nullptr : &subresource_data[0], &texture_handle_3d);
            ASSERT(result == S_OK);

            break;
        }
        case TT_CUBEMAP: {
            // Create texture
            D3D11_TEXTURE2D_DESC texture_desc = {};
            texture_desc.Width            = (u32)desc.width;
            texture_desc.Height           = (u32)desc.height;
            texture_desc.MipLevels        = desc.mipmap_count;
            texture_desc.Format           = texture_format;
            texture_desc.SampleDesc.Count = 1;
            texture_desc.Usage            = D3D11_USAGE_DEFAULT;
            texture_desc.ArraySize        = 6;
            texture_desc.MiscFlags        |= D3D11_RESOURCE_MISC_TEXTURECUBE;

            if (texture_format_infos[desc.format].is_depth_format) {
                texture_desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
            }
            else {
                texture_desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
                if (desc.uav) {
                    texture_desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
                }
                if (desc.render_target) {
                    texture_desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
                }
            }

            u32 pixel_size = (u32)texture_format_infos[desc.format].pixel_size_in_bytes;
            D3D11_SUBRESOURCE_DATA subresource_data[6] = {
                {desc.color_data, pixel_size * (u32)desc.width,    0},
                {desc.color_data, pixel_size * (u32)desc.width/2,  0},
                {desc.color_data, pixel_size * (u32)desc.width/4,  0},
                {desc.color_data, pixel_size * (u32)desc.width/8,  0},
                {desc.color_data, pixel_size * (u32)desc.width/16, 0},
                {desc.color_data, pixel_size * (u32)desc.width/32, 0},
            };

            auto result = directx.device->CreateTexture2D(&texture_desc, desc.color_data == nullptr ? nullptr : &subresource_data[0], &texture_handle_2d);
            ASSERT(result == S_OK);

            if (desc.sample_count > 1) {
                ASSERT(desc.render_target);
                ASSERT(!desc.uav);
                D3D11_TEXTURE2D_DESC msaa_texture_desc = texture_desc;
                msaa_texture_desc.SampleDesc.Count = desc.sample_count;
                msaa_texture_desc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
                result = directx.device->CreateTexture2D(&msaa_texture_desc, nullptr, &msaa_handle_2d);
                ASSERT(result == S_OK);
            }

            break;
        }
    }

    Texture texture = {};
    texture.valid = true;
    texture.description = desc;
    texture.backend.handle_2d = texture_handle_2d;
    texture.backend.handle_msaa_2d = msaa_handle_2d;
    texture.backend.handle_3d = texture_handle_3d;
    texture.backend.shader_resource_view = shader_resource_view;
    texture.backend.uav = uav;
    return texture;
}

void destroy_texture(Texture texture) {
    if (texture.backend.handle_2d != nullptr) {
        texture.backend.handle_2d->Release();
    }
    if (texture.backend.handle_msaa_2d != nullptr) {
        texture.backend.handle_msaa_2d->Release();
    }
    if (texture.backend.handle_3d != nullptr) {
        texture.backend.handle_3d->Release();
    }
    if (texture.backend.shader_resource_view) {
        texture.backend.shader_resource_view->Release();
    }
    if (texture.backend.uav) {
        texture.backend.uav->Release();
    }
}

void set_cubemap_textures(Texture texture, byte *face_pixel_data[6]) {
    ASSERT(texture.description.type == TT_CUBEMAP);

    u32 num_channels = (u32)texture_format_infos[texture.description.format].num_channels;
    for (int idx = 0; idx < 6; idx++) {
        byte *face_data = face_pixel_data[idx];
        if (face_data != nullptr) {
            directx.device_context->UpdateSubresource((ID3D11Resource *)texture.backend.handle_2d, (u32)idx, nullptr, face_data, num_channels * (u32)texture.description.width, 0);
        }
    }
}

void *map_texture(Texture *texture) {
    D3D11_MAPPED_SUBRESOURCE texture_resource = {};
    auto result = directx.device_context->Map(*((ID3D11Resource **)&texture->backend.handle_2d), 0, D3D11_MAP_READ, 0, &texture_resource);
    assert(result == S_OK);
    return texture_resource.pData;
}

void unmap_texture(Texture *texture) {
    directx.device_context->Unmap(*((ID3D11Resource **)&texture->backend.handle_2d), 0);
}

ID3D11ShaderResourceView *dx_create_shader_resource_view(Texture texture) {
    ASSERT(!texture_format_infos[texture.description.format].is_depth_format);
    ID3D11ShaderResourceView *shader_resource_view = {};
    switch (texture.description.type) {
        case TT_2D: {
            D3D11_SHADER_RESOURCE_VIEW_DESC texture_shader_resource_desc = {};
            texture_shader_resource_desc.Format = dx_texture_format_mapping[texture.description.format];
            texture_shader_resource_desc.Texture2D.MipLevels = texture.description.mipmap_count;
            texture_shader_resource_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            auto result = directx.device->CreateShaderResourceView((ID3D11Resource *)texture.backend.handle_2d, &texture_shader_resource_desc, &shader_resource_view);
            ASSERT(result == S_OK);
            break;
        }
        case TT_3D: {
            D3D11_SHADER_RESOURCE_VIEW_DESC texture_shader_resource_desc = {};
            texture_shader_resource_desc.Format = dx_texture_format_mapping[texture.description.format];
            texture_shader_resource_desc.Texture3D.MipLevels = texture.description.mipmap_count;
            texture_shader_resource_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
            auto result = directx.device->CreateShaderResourceView((ID3D11Resource *)texture.backend.handle_3d, &texture_shader_resource_desc, &shader_resource_view);
            ASSERT(result == S_OK);
            break;
        }
        case TT_CUBEMAP: {
            D3D11_SHADER_RESOURCE_VIEW_DESC texture_shader_resource_desc = {};
            texture_shader_resource_desc.Format = dx_texture_format_mapping[texture.description.format];
            texture_shader_resource_desc.TextureCube.MipLevels = texture.description.mipmap_count;
            texture_shader_resource_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
            auto result = directx.device->CreateShaderResourceView((ID3D11Resource *)texture.backend.handle_2d, &texture_shader_resource_desc, &shader_resource_view);
            ASSERT(result == S_OK);
            break;
        }
        default: {
            ASSERT(false);
        }
    }
    ASSERT(shader_resource_view != nullptr);
    return shader_resource_view;
}

void bind_texture(Texture texture, int slot) {
    ASSERT(slot < CFF_MAX_BOUND_TEXTURES);
    if (directx.cur_srvs[slot]) {
        directx.cur_srvs[slot]->Release();
        directx.cur_srvs[slot] = {};
    }

    if (texture.valid) {
        directx.cur_srvs[slot] = dx_create_shader_resource_view(texture);
        switch (texture.description.wrap_mode) {
            case TWM_LINEAR_WRAP:  directx.device_context->PSSetSamplers(0, 1, &directx.linear_wrap_sampler);  break;
            case TWM_LINEAR_CLAMP: directx.device_context->PSSetSamplers(0, 1, &directx.linear_clamp_sampler); break;
            case TWM_POINT_WRAP:   directx.device_context->PSSetSamplers(0, 1, &directx.point_wrap_sampler);   break;
            case TWM_POINT_CLAMP:  directx.device_context->PSSetSamplers(0, 1, &directx.point_clamp_sampler);  break;
            default: {
                ASSERT(false && "No wrap mode specified for texture.");
            }
        }
    }
    directx.device_context->PSSetShaderResources(slot, 1, &directx.cur_srvs[slot]);
}

void unbind_all_textures() {
    for (int i = 0; i < ARRAYSIZE(directx.cur_srvs); i++) {
        if (directx.cur_srvs[i]) {
            directx.cur_srvs[i]->Release();
            directx.cur_srvs[i] = {};
        }
    }
    directx.device_context->PSSetShaderResources(0, ARRAYSIZE(directx.cur_srvs), &directx.cur_srvs[0]);
}

void copy_texture(Texture dst, Texture src) {
    directx.device_context->CopyResource((ID3D11Resource *)dst.backend.handle_2d, (ID3D11Resource *)src.backend.handle_2d);
}

void set_render_targets(Render_Target_Bindings bindings) {
    unset_render_targets();

    for (int idx = 0; idx < ARRAYSIZE(bindings.color_bindings); idx++) {
        Texture color_buffer = bindings.color_bindings[idx].texture;
        bool clear = bindings.color_bindings[idx].clear;
        Vector4 clear_color = bindings.color_bindings[idx].clear_color;

        ASSERT(directx.cur_rtvs[idx] == nullptr);

        if (color_buffer.valid) {
            ASSERT(color_buffer.description.type == TT_2D);
            ASSERT(color_buffer.description.render_target);
            directx.current_render_targets[idx] = color_buffer;
            bool msaa = color_buffer.description.sample_count > 1;
            if (msaa) {
                ASSERT(color_buffer.backend.handle_msaa_2d != nullptr);
            }
            directx.cur_rtvs[idx] = dx_create_render_target_view(msaa ? color_buffer.backend.handle_msaa_2d : color_buffer.backend.handle_2d, color_buffer.description.format, msaa);
        }
        else {
            if (idx == 0) {
                // bind the swapchain as the render target
                ID3D11Texture2D *back_buffer_texture = {};
                auto result = directx.swap_chain_handle->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&back_buffer_texture);
                ASSERT(result == S_OK);
                ASSERT(directx.cur_rtvs[0] == nullptr);
                directx.cur_rtvs[0] = dx_create_render_target_view(back_buffer_texture, SWAP_CHAIN_FORMAT, false); // todo(josh): swap chain msaa
                back_buffer_texture->Release();
            }
        }

        if (directx.cur_rtvs[idx] && clear) {
            float color_elements[4] = { clear_color.x, clear_color.y, clear_color.z, clear_color.w };
            directx.device_context->ClearRenderTargetView(directx.cur_rtvs[idx], color_elements);
        }
    }

    Depth_Buffer_Binding depth_binding = bindings.depth_binding;
    Texture *depth_buffer_to_use = &directx.swap_chain_depth_buffer;
    if (depth_binding.texture.valid) {
        depth_buffer_to_use = &depth_binding.texture;;
    }

    ASSERT(directx.cur_dsv == nullptr);
    ASSERT(depth_buffer_to_use != nullptr);
    ASSERT(depth_buffer_to_use->description.type == TT_2D);
    ASSERT(depth_buffer_to_use->description.render_target);
    bool depth_msaa = depth_buffer_to_use->description.sample_count > 1;
    directx.cur_dsv = dx_create_depth_stencil_view(depth_msaa ? depth_buffer_to_use->backend.handle_msaa_2d : depth_buffer_to_use->backend.handle_2d, depth_buffer_to_use->description.format, depth_msaa);
    if (depth_binding.clear) {
        directx.device_context->ClearDepthStencilView(directx.cur_dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth_binding.clear_depth, 0);
    }

    directx.device_context->OMSetRenderTargets(CFF_MAX_BOUND_RENDER_TARGETS, &directx.cur_rtvs[0], directx.cur_dsv);
}

void unset_render_targets() {
    for (int i = 0; i < CFF_MAX_BOUND_RENDER_TARGETS; i++) {
        ID3D11RenderTargetView *rtv = directx.cur_rtvs[i];
        if (directx.cur_rtvs[i] != nullptr) {
            Texture target = directx.current_render_targets[i];
            if (target.valid) {
                ASSERT(target.description.type == TT_2D);
                ASSERT(target.description.render_target);
                if (target.backend.handle_msaa_2d != nullptr) {
                    directx.device_context->ResolveSubresource((ID3D11Resource *)target.backend.handle_2d, 0, (ID3D11Resource *)target.backend.handle_msaa_2d, 0, dx_texture_format_mapping[target.description.format]);
                }
                directx.current_render_targets[i] = {};
            }

            directx.cur_rtvs[i]->Release();
            directx.cur_rtvs[i] = nullptr;
        }
    }
    if (directx.cur_dsv != nullptr) {
        directx.cur_dsv->Release();
        directx.cur_dsv = nullptr;
    }
}

// void clear_bound_render_targets(Vector4 color) {
//     for (int i = 0; i < CFF_MAX_BOUND_RENDER_TARGETS; i++) {
//         ID3D11RenderTargetView *rtv = directx.cur_rtvs[i];
//         if (rtv != nullptr) {
//             float color_elements[4] = { color.x, color.y, color.z, color.w };
//             directx.device_context->ClearRenderTargetView(rtv, color_elements);
//         }
//     }
//     if (directx.cur_dsv != nullptr) {
//         directx.device_context->ClearDepthStencilView(directx.cur_dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
//     }
// }

void present(bool vsync) {
    directx.swap_chain_handle->Present(vsync, 0);
}

#endif

#endif

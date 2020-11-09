#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "window.h"
#include "basic.h"
#include "math.h"
#include "renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static Window g_main_window;
static float g_time_at_startup;

struct Pass_CBuffer {
    Matrix4 view_matrix;
    Matrix4 projection_matrix;
};

struct Model_CBuffer {
    Matrix4 model_matrix;
};

struct Vertex {
    Vector3 position;
    Vector3 tex_coord;
    Vector4 color;
};

void main() {
    init_platform();

    g_time_at_startup = time_now();

    Allocator global_allocator = default_allocator();

    g_main_window = create_window(1920, 1080);

    init_render_backend(&g_main_window);
    init_renderer(&g_main_window);

    Vertex_Shader vertex_shader = compile_vertex_shader_from_file(L"vertex.hlsl");
    Pixel_Shader pixel_shader = compile_pixel_shader_from_file(L"pixel.hlsl");

    int filedata_len;
    char *filedata = read_entire_file("my_texture.png", &filedata_len);
    // stbi_set_flip_vertically_on_load(1);
    int x,y,n;
    byte *color_data = stbi_load_from_memory((byte *)filedata, filedata_len, &x, &y, &n, 4);
    assert(color_data);

    Texture_Description texture_description = {};
    texture_description.width = x;
    texture_description.height = y;
    texture_description.color_data = color_data;
    texture_description.format = TF_R8G8B8A8_UINT;
    texture_description.type = TT_2D;
    Texture texture = create_texture(texture_description);
    stbi_image_free(color_data);
    free(filedata);

    // Make vertex format
    Vertex_Field vertex_fields[] = {
        {"POSITION", "position",  offsetof(Vertex, position),  VFT_FLOAT3, VFST_PER_VERTEX},
        {"TEXCOORD", "tex_coord", offsetof(Vertex, tex_coord), VFT_FLOAT3, VFST_PER_VERTEX},
        {"COLOR",    "color",     offsetof(Vertex, color),     VFT_FLOAT4, VFST_PER_VERTEX},
    };
    Vertex_Format default_vertex_format = create_vertex_format(vertex_fields, ARRAYSIZE(vertex_fields));

    Buffer pass_cbuffer_handle  = create_buffer(BT_CONSTANT, nullptr, sizeof(Pass_CBuffer));
    Buffer model_cbuffer_handle = create_buffer(BT_CONSTANT, nullptr, sizeof(Model_CBuffer));

    u32 cube_indices[] = {
         0,  2,  1,  0,  3,  2,
         4,  5,  6,  4,  6,  7,
         8, 10,  9,  8, 11, 10,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 22, 21, 20, 23, 22,
    };

    FFVertex cube_vertices[] = {
        {{-0.5f, -0.5f, -0.5f}, {}, {}},
        {{ 0.5f, -0.5f, -0.5f}, {}, {}},
        {{ 0.5f,  0.5f, -0.5f}, {}, {}},
        {{-0.5f,  0.5f, -0.5f}, {}, {}},

        {{-0.5f, -0.5f,  0.5f}, {}, {}},
        {{ 0.5f, -0.5f,  0.5f}, {}, {}},
        {{ 0.5f,  0.5f,  0.5f}, {}, {}},
        {{-0.5f,  0.5f,  0.5f}, {}, {}},

        {{-0.5f, -0.5f, -0.5f}, {}, {}},
        {{-0.5f,  0.5f, -0.5f}, {}, {}},
        {{-0.5f,  0.5f,  0.5f}, {}, {}},
        {{-0.5f, -0.5f,  0.5f}, {}, {}},

        {{ 0.5f, -0.5f, -0.5f}, {}, {}},
        {{ 0.5f,  0.5f, -0.5f}, {}, {}},
        {{ 0.5f,  0.5f,  0.5f}, {}, {}},
        {{ 0.5f, -0.5f,  0.5f}, {}, {}},

        {{-0.5f, -0.5f, -0.5f}, {}, {}},
        {{ 0.5f, -0.5f, -0.5f}, {}, {}},
        {{ 0.5f, -0.5f,  0.5f}, {}, {}},
        {{-0.5f, -0.5f,  0.5f}, {}, {}},

        {{-0.5f,  0.5f, -0.5f}, {}, {}},
        {{ 0.5f,  0.5f, -0.5f}, {}, {}},
        {{ 0.5f,  0.5f,  0.5f}, {}, {}},
        {{-0.5f,  0.5f,  0.5f}, {}, {}},
    };

    Buffer cube_vertex_buffer = create_buffer(BT_VERTEX, cube_vertices, sizeof(cube_vertices));
    Buffer cube_index_buffer  = create_buffer(BT_INDEX,  cube_indices,  sizeof(cube_indices));

    Vector3 camera_position = {};
    Quaternion camera_rotation = quaternion_identity();

    while (true) {
        update_window(&g_main_window);

        if (get_input(&g_main_window, INPUT_E)) camera_position.y += 0.1f;
        if (get_input(&g_main_window, INPUT_Q)) camera_position.y -= 0.1f;

        Vector2 delta = g_main_window.mouse_position_pixel_delta * 0.25f;

        // rotate quat by degrees
        Quaternion yaw   = quaternion_identity() * axis_angle(v3(0, 1, 0), to_radians(delta.x));
        Quaternion pitch = quaternion_identity() * axis_angle(v3(1, 0, 0), to_radians(delta.y));
        camera_rotation = pitch * camera_rotation * yaw;

        prerender(g_main_window.width, g_main_window.height);

        bind_vertex_format(default_vertex_format);
        bind_shaders(vertex_shader, pixel_shader);
        bind_textures(&texture, 0, 1);

        Vector3 cube_position = v3(5, 5, 10);

        Pass_CBuffer pass_cbuffer = {};
        pass_cbuffer.view_matrix = view_matrix(camera_position, camera_rotation);
        pass_cbuffer.projection_matrix = perspective(to_radians(60), (float)g_main_window.width / (float)g_main_window.height, 0.001, 1000);
        update_buffer(pass_cbuffer_handle, &pass_cbuffer, sizeof(Pass_CBuffer));
        bind_constant_buffers(&pass_cbuffer_handle, 1, 0);

        Model_CBuffer model_cbuffer = {};
        model_cbuffer.model_matrix = model_matrix(cube_position, v3(1, 1, 1), quaternion_identity());
        update_buffer(model_cbuffer_handle, &model_cbuffer, sizeof(Model_CBuffer));
        bind_constant_buffers(&model_cbuffer_handle, 1, 1);

        // FFVertex vertices[1024] = {};
        // Fixed_Function ff = {};
        // ff_begin(&ff, vertices, ARRAYSIZE(vertices));
        // ff_vertex(&ff, v3(-0.5f, -0.5f, 0)); ff_tex_coord(&ff, v3(0, 1, 0)); ff_color(&ff, v4(1, 1, 1, 1)); ff_next(&ff);
        // ff_vertex(&ff, v3(-0.5f,  0.5f, 0)); ff_tex_coord(&ff, v3(0, 0, 0)); ff_color(&ff, v4(1, 1, 1, 1)); ff_next(&ff);
        // ff_vertex(&ff, v3( 0.5f, -0.5f, 0)); ff_tex_coord(&ff, v3(1, 1, 0)); ff_color(&ff, v4(1, 1, 1, 1)); ff_next(&ff);
        // ff_vertex(&ff, v3( 0.5f,  0.5f, 0)); ff_tex_coord(&ff, v3(1, 0, 0)); ff_color(&ff, v4(1, 1, 1, 1)); ff_next(&ff);
        // ff_vertex(&ff, v3( 0.5f, -0.5f, 0)); ff_tex_coord(&ff, v3(1, 1, 0)); ff_color(&ff, v4(1, 1, 1, 1)); ff_next(&ff);
        // ff_vertex(&ff, v3(-0.5f,  0.5f, 0)); ff_tex_coord(&ff, v3(0, 0, 0)); ff_color(&ff, v4(1, 1, 1, 1)); ff_next(&ff);
        // ff_end(&ff);

        u32 strides[1] = {sizeof(FFVertex)};
        u32 offsets[1] = {0};
        bind_vertex_buffers(&cube_vertex_buffer, 1, 0, strides, offsets);
        bind_index_buffer(cube_index_buffer, 0);
        issue_draw_call(ARRAYSIZE(cube_vertices), ARRAYSIZE(cube_indices));

        present(true);
    }

    Fixed_Function ff = {};

}
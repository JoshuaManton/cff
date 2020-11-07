#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "basic.h"
#include "window.h"
#include "math.h"
#include "renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static Window g_main_window;
static float g_time_at_startup;

void main() {
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

    while (true) {
        update_window();
        prerender(g_main_window.width, g_main_window.height);

        bind_shaders(vertex_shader, pixel_shader);

        bind_textures(&texture, 0, 1);

        FFVertex vertices[1024] = {};
        Fixed_Function ff = {};
        ff_begin(&ff, vertices, ARRAYSIZE(vertices));
        ff_vertex(&ff, v3(-0.5f, -0.5f, 0)); ff_tex_coord(&ff, v3(0, 1, 0)); ff_color(&ff, v4(1, 1, 1, 1)); ff_next(&ff);
        ff_vertex(&ff, v3(-0.5f,  0.5f, 0)); ff_tex_coord(&ff, v3(0, 0, 0)); ff_color(&ff, v4(1, 1, 1, 1)); ff_next(&ff);
        ff_vertex(&ff, v3( 0.5f, -0.5f, 0)); ff_tex_coord(&ff, v3(1, 1, 0)); ff_color(&ff, v4(1, 1, 1, 1)); ff_next(&ff);
        ff_vertex(&ff, v3( 0.5f,  0.5f, 0)); ff_tex_coord(&ff, v3(1, 0, 0)); ff_color(&ff, v4(1, 1, 1, 1)); ff_next(&ff);
        ff_vertex(&ff, v3( 0.5f, -0.5f, 0)); ff_tex_coord(&ff, v3(1, 1, 0)); ff_color(&ff, v4(1, 1, 1, 1)); ff_next(&ff);
        ff_vertex(&ff, v3(-0.5f,  0.5f, 0)); ff_tex_coord(&ff, v3(0, 0, 0)); ff_color(&ff, v4(1, 1, 1, 1)); ff_next(&ff);
        ff_end(&ff);

        present(true);
    }

    Fixed_Function ff = {};

}
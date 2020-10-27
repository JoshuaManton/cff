#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "basic.h"
#define STB_IMAGE_IMPLEMENTATION
#include "window.h"
#include "stb_image.h"
#include "math.h"
#include "renderer.h"

struct Fixed_Function {
    Vertex *vertices;
    int max_vertices;
    int num_vertices;
};

void ff_begin(Fixed_Function *ff, Vertex *buffer, int max_vertices) {
    ff->num_vertices = 0;
    ff->vertices = buffer;
    ff->max_vertices = max_vertices;
}

void ff_end(Fixed_Function *ff) {
    assert(ff->num_vertices < ff->max_vertices);
    Buffer vertex_buffer = create_vertex_buffer(ff->vertices, ff->num_vertices);
    bind_vertex_buffers(&vertex_buffer, 1);
    draw(ff->num_vertices, 0);
    destroy_vertex_buffer(vertex_buffer);
}

void ff_vertex(Fixed_Function *ff, Vector3 position) {
    assert(ff->num_vertices < ff->max_vertices);
    ff->vertices[ff->num_vertices].position = v3(position.x, position.y, position.z);
}

void ff_tex_coord(Fixed_Function *ff, Vector3 tex_coord) {
    assert(ff->num_vertices < ff->max_vertices);
    ff->vertices[ff->num_vertices].tex_coord = tex_coord;
}

void ff_color(Fixed_Function *ff, Vector4 color) {
    assert(ff->num_vertices < ff->max_vertices);
    ff->vertices[ff->num_vertices].color = color;
}

void ff_next(Fixed_Function *ff) {
    ff->num_vertices += 1;
}

static Window main_window;
static float time_at_startup;

void main() {
    time_at_startup = time_now();

    Allocator global_allocator = default_allocator();

    main_window = create_window(1920, 1080);

    init_render_backend(&main_window);

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
    Texture texture = create_texture(texture_description);
    stbi_image_free(color_data);
    free(filedata);

    while (true) {
        update_window();
        prerender(main_window.width, main_window.height);

        bind_shaders(vertex_shader, pixel_shader);

        bind_textures(&texture, 0, 1);

        Vertex vertices[1024] = {};
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
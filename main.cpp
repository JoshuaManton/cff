#include <stdio.h>
#include <stdlib.h>

#include "basic.cpp"
#include "math.cpp"
#include "types.cpp"
#include "allocators.cpp"
#include "window.cpp"
#include "directx.cpp"

struct Fixed_Function {
    Vertex vertices[64];
    int num_vertices;
};

void ff_begin(Fixed_Function *ff) {
    ff->num_vertices = 0;
}

void ff_end(Fixed_Function *ff) {
    assert(ff->num_vertices < ARRAYSIZE(ff->vertices));
    Vertex_Buffer vertex_buffer = create_vertex_buffer(ff->vertices, ff->num_vertices);
    bind_vertex_buffers(&vertex_buffer, 1);
    draw(ff->num_vertices, 0);
    destroy_vertex_buffer(vertex_buffer);
}

void ff_vertex(Fixed_Function *ff, Vector3 position) {
    assert(ff->num_vertices < ARRAYSIZE(ff->vertices));
    ff->vertices[ff->num_vertices].position = v3(position.x, position.y, position.z);
}

void ff_tex_coord(Fixed_Function *ff, Vector3 tex_coord) {
    assert(ff->num_vertices < ARRAYSIZE(ff->vertices));
    ff->vertices[ff->num_vertices].tex_coord = tex_coord;
}

void ff_color(Fixed_Function *ff, Vector4 color) {
    assert(ff->num_vertices < ARRAYSIZE(ff->vertices));
    ff->vertices[ff->num_vertices].color = color;
}

void ff_next(Fixed_Function *ff) {
    ff->num_vertices += 1;
}

void main() {
    time_at_startup = time_now();

    Allocator global_allocator = default_allocator();

    create_window(1920, 1080);

    init_directx();

    Vertex_Shader vertex_shader = compile_vertex_shader_from_file(L"vertex.hlsl");
    Pixel_Shader pixel_shader = compile_pixel_shader_from_file(L"pixel.hlsl");

    byte color_data[16] = {
        255, 0, 0, 255,
        0, 255, 0, 255,
        0, 0, 255, 255,
        255, 255, 255, 255,
    };

    Texture_Description texture_description = {};
    texture_description.width = 2;
    texture_description.height = 2;
    texture_description.color_data = color_data;
    Texture texture = create_texture(texture_description);

    while (true) {
        update_window();
        prerender();

        bind_shaders(vertex_shader, pixel_shader);

        bind_textures(&texture, 0, 1);

        Fixed_Function ff = {};
        ff_begin(&ff);
        ff_vertex(&ff, v3(-0.5f, -0.5f, 0)); ff_tex_coord(&ff, v3(0, 1, 0)); ff_color(&ff, v4(1, 1, 1, 1)); ff_next(&ff);
        ff_vertex(&ff, v3(-0.5f,  0.5f, 0)); ff_tex_coord(&ff, v3(0, 0, 0)); ff_color(&ff, v4(1, 1, 1, 1)); ff_next(&ff);
        ff_vertex(&ff, v3( 0.5f, -0.5f, 0)); ff_tex_coord(&ff, v3(1, 1, 0)); ff_color(&ff, v4(1, 1, 1, 1)); ff_next(&ff);
        ff_vertex(&ff, v3( 0.5f,  0.5f, 0)); ff_tex_coord(&ff, v3(1, 0, 0)); ff_color(&ff, v4(1, 1, 1, 1)); ff_next(&ff);
        ff_vertex(&ff, v3( 0.5f, -0.5f, 0)); ff_tex_coord(&ff, v3(1, 1, 0)); ff_color(&ff, v4(1, 1, 1, 1)); ff_next(&ff);
        ff_vertex(&ff, v3(-0.5f,  0.5f, 0)); ff_tex_coord(&ff, v3(0, 0, 0)); ff_color(&ff, v4(1, 1, 1, 1)); ff_next(&ff);
        ff_end(&ff);

        postrender();
    }

    Fixed_Function ff = {};

}
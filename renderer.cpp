#include "renderer.h"

struct Renderer_State {
    Vertex_Format ff_vertex_format;
};

static Renderer_State renderer_state;

void init_renderer(Window *window) {
    // Make vertex format
    Vertex_Field fields[] = {
        {"SV_POSITION", "position",  offsetof(FFVertex, position),  VFT_FLOAT3, VFST_PER_VERTEX},
        {"TEXCOORD",    "tex_coord", offsetof(FFVertex, tex_coord), VFT_FLOAT3, VFST_PER_VERTEX},
        {"COLOR",       "color",     offsetof(FFVertex, color),     VFT_FLOAT4, VFST_PER_VERTEX},
    };
    renderer_state.ff_vertex_format = create_vertex_format(fields, ARRAYSIZE(fields));
}

void ff_begin(Fixed_Function *ff, FFVertex *buffer, int max_vertices) {
    ff->num_vertices = 0;
    ff->vertices = buffer;
    ff->max_vertices = max_vertices;
}

void ff_end(Fixed_Function *ff) {
    assert(ff->num_vertices < ff->max_vertices);
    bind_vertex_format(renderer_state.ff_vertex_format);

    // todo(josh): should this create() go in ff_begin?
    Buffer vertex_buffer = create_buffer(BT_VERTEX, ff->vertices, sizeof(ff->vertices[0]) * ff->num_vertices);
    u32 strides[1] = { sizeof(ff->vertices[0]) };
    u32 offsets[1] = { 0 };
    bind_vertex_buffers(&vertex_buffer, 1, 0, strides, offsets);
    issue_draw_call(ff->num_vertices, 0);
    destroy_buffer(vertex_buffer);
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
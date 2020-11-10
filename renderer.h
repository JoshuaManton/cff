#pragma once

#include "window.h"
#include "math.h"
#include "render_backend.h"

struct Pass_CBuffer {
    Matrix4 view_matrix;
    Matrix4 projection_matrix;
    Vector3 camera_position;
    f32 pad0;
};

struct Model_CBuffer {
    Matrix4 model_matrix;
};

struct Vertex {
    Vector3 position;
    Vector3 tex_coord;
    Vector4 color;
};

struct FFVertex {
    Vector3 position;
    Vector3 tex_coord;
    Vector4 color;
};

struct Fixed_Function {
    FFVertex *vertices;
    int max_vertices;
    int num_vertices;
};

void ff_begin(Fixed_Function *ff, FFVertex *buffer, int max_vertices);
void ff_end(Fixed_Function *ff);
void ff_vertex(Fixed_Function *ff, Vector3 position);
void ff_tex_coord(Fixed_Function *ff, Vector3 tex_coord);
void ff_color(Fixed_Function *ff, Vector4 color);
void ff_next(Fixed_Function *ff);
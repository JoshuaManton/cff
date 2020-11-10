#pragma once

#include "window.h"
#include "math.h"
#include "render_backend.h"

struct Vertex {
    Vector3 position;
    Vector3 tex_coord;
    Vector4 color;
    Vector3 normal;
    Vector3 tangent;
    Vector3 bitangent;
};

struct Pass_CBuffer {
    Matrix4 view_matrix;
    Matrix4 projection_matrix;
    Vector3 camera_position;
    f32 pad0;
};

struct Model_CBuffer {
    Matrix4 model_matrix;
    int has_albedo_map;
    int has_normal_map;
    int has_metallic_map;
    int has_roughness_map;
    int has_emission_map;
    int has_ao_map;
    float ambient;
    float metallic;
    float roughness;
    float pad0;
    float pad1;
    float pad2;
};

struct Material {
    Texture albedo_map;
    Texture normal_map;
    Texture metallic_map;
    Texture roughness_map;
    Texture emission_map;
    Texture ao_map;
    float ambient;
    float metallic;
    float roughness;
};

struct Loaded_Mesh {
    Buffer vertex_buffer;
    int num_vertices;
    Buffer index_buffer;
    int num_indices;
    Material material;
    bool has_material;
};

enum CBuffer_Slot {
    CBS_PASS,
    CBS_MODEL,
    CBS_LIGHTING,
};

enum Texture_Slot {
    TS_ALBEDO,
    TS_NORMAL,
    TS_METALLIC,
    TS_ROUGHNESS,
    TS_EMISSION,
    TS_AO,
    TS_COUNT,
};

#define MAX_POINT_LIGHTS 16 // :MaxPointLights
struct Lighting_CBuffer {
    Vector4 point_light_positions[MAX_POINT_LIGHTS];
    Vector4 point_light_colors[MAX_POINT_LIGHTS];
    int num_point_lights;
    float pad[3];
};

struct Fixed_Function {
    Vertex *vertices;
    int max_vertices;
    int num_vertices;
};

Texture load_texture_from_file(char *filename, Texture_Format format = TF_R8G8B8A8_UINT);

struct Render_Options {
    bool do_albedo;
    bool do_normal;
    bool do_metallic;
    bool do_roughness;
    bool do_emission;
    bool do_ao;
};

struct Render_Pass_Desc {
    Vector3 camera_position;
    Quaternion camera_orientation;
    Matrix4 projection_matrix;
};

void begin_render_pass(Render_Pass_Desc *pass);
void draw_meshes(Array<Loaded_Mesh> meshes, Vector3 position, Vector3 scale, Quaternion orientation, Render_Options options);

void ff_begin(Fixed_Function *ff, Vertex *buffer, int max_vertices);
void ff_end(Fixed_Function *ff);
void ff_vertex(Fixed_Function *ff, Vector3 position);
void ff_tex_coord(Fixed_Function *ff, Vector3 tex_coord);
void ff_color(Fixed_Function *ff, Vector4 color);
void ff_next(Fixed_Function *ff);

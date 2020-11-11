#pragma once

#include "window.h"
#include "math.h"
#include "render_backend.h"
#include "stb_truetype.h"

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
    int visualize_normals;
    float pad0;
    float pad1;
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
    bool has_transparency;
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
    CBS_PASS     = 0,
    CBS_MODEL    = 1,
    CBS_LIGHTING = 2,
    CBS_BLUR     = 2,
};

enum Texture_Slot {
    TS_ALBEDO = 0,
    TS_NORMAL = 1,
    TS_FINAL_BLOOM_MAP = 1,
    TS_METALLIC = 2,
    TS_ROUGHNESS = 3,
    TS_EMISSION = 4,
    TS_AO = 5,
    TS_SHADOW_MAP = 6,
};

#define MAX_POINT_LIGHTS 16 // :MaxPointLights
struct Lighting_CBuffer {
    Vector4 point_light_positions[MAX_POINT_LIGHTS];
    Vector4 point_light_colors[MAX_POINT_LIGHTS];
    int num_point_lights;
    float pad1[3];
    Matrix4 sun_transform;
    Vector3 sun_direction;
    float pad2;
    Vector3 sun_color;
    float pad3;
};

Texture load_texture_from_file(char *filename, Texture_Format format = TF_R8G8B8A8_UINT);

struct Font {
    Texture texture;
    stbtt_bakedchar chars[128];
    int dim;
    float pixel_height;
};

Font load_font_from_file(char *filename, float size);
void destroy_font(Font font);

struct Render_Options {
    bool do_albedo_map;
    bool do_normal_map;
    bool do_metallic_map;
    bool do_roughness_map;
    bool do_emission_map;
    bool do_ao_map;
    bool visualize_normals;
};

struct Render_Pass_Desc {
    Vector3 camera_position;
    Quaternion camera_orientation;
    Matrix4 projection_matrix;
};

void begin_render_pass(Render_Pass_Desc *pass);
void draw_meshes(Array<Loaded_Mesh> meshes, Vector3 position, Vector3 scale, Quaternion orientation, Render_Options options, bool draw_transparency);

struct Fixed_Function {
    Vertex *vertices;
    int max_vertices;
    int num_vertices;
    Texture texture;
    Vertex_Shader vertex_shader;
    Pixel_Shader pixel_shader;
};

void ff_begin(Fixed_Function *ff, Vertex *buffer, int max_vertices, Texture texture, Vertex_Shader vertex_shader, Pixel_Shader pixel_shader);
void ff_end(Fixed_Function *ff);
void ff_vertex(Fixed_Function *ff, Vector3 position);
void ff_tex_coord(Fixed_Function *ff, Vector3 tex_coord);
void ff_color(Fixed_Function *ff, Vector4 color);
void ff_next(Fixed_Function *ff);
void ff_quad(Fixed_Function *ff, Vector3 min, Vector3 max, Vector4 color, Vector2 uv_overrides[2]);
void ff_text(Fixed_Function *ff, char *str, Font font, Vector4 color, Vector3 start_pos, float size);
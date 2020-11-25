#pragma once

#include "application.h"
#include "math.h"
#include "stb_truetype.h"

void init_renderer(Window *window);

struct Vertex {
    Vector3 position;
    Vector3 tex_coord;
    Vector4 color;
    Vector3 normal;
    Vector3 tangent;
    Vector3 bitangent;
};

struct PBR_Material {
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
    Buffer cbuffer_handle;
};

void flush_pbr_material(Buffer buffer, PBR_Material material);
Buffer create_pbr_material_cbuffer();

struct Loaded_Mesh {
    Buffer vertex_buffer;
    int num_vertices;
    Buffer index_buffer;
    int num_indices;
    PBR_Material material;
    bool has_material;
};

struct Model {
    Array<Loaded_Mesh> meshes;
};

Model create_model(Allocator allocator);

struct Pass_CBuffer {
    Vector2 screen_dimensions;
    f32 pad[2];
    Matrix4 view_matrix;
    Matrix4 projection_matrix;
    Vector3 camera_position;
    f32 pad2;
};

struct Model_CBuffer {
    Matrix4 model_matrix;
    Vector4 model_color;
};

struct PBR_Material_CBuffer {
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
    float fog_y_level;
    float fog_density;
    Vector3 fog_base_color;
    int has_skybox_map;
    float pad3[3];
    Vector4 skybox_color;
};



#define CBS_PASS     0
#define CBS_MODEL    1
#define CBS_MATERIAL 2
#define CBS_LIGHTING 3
#define CBS_BLUR     4
#define CBS_FINAL    5
#define CBS_SSR      6



#define TS_PBR_ALBEDO        0
#define TS_PBR_NORMAL        1
#define TS_PBR_METALLIC      2
#define TS_PBR_ROUGHNESS     3
#define TS_PBR_EMISSION      4
#define TS_PBR_AO            5
#define TS_PBR_SHADOW_MAP    6
#define TS_PBR_DEPTH_PREPASS 7
#define TS_PBR_SKYBOX        8

#define TS_FINAL_MAIN_VIEW 0
#define TS_FINAL_BLOOM_MAP 1
#define TS_FINAL_SSR_MAP   2

#define TS_SIMPLE_ALBEDO 0

#define TS_SSR_SCENE_MAP           0
#define TS_SSR_NORMAL_MAP          1
#define TS_SSR_POSITIONS_MAP       2
#define TS_SSR_METAL_ROUGHNESS_MAP 3



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
    Render_Target_Bindings render_target_bindings;
    Vector3 camera_position;
    Quaternion camera_orientation;
    Matrix4 projection_matrix;
};

void begin_render_pass(Render_Pass_Desc *pass);
void end_render_pass();
void draw_mesh(Buffer vertex_buffer, Buffer index_buffer, int num_vertices, int num_indices, Vector3 position, Vector3 scale, Quaternion orientation, Vector4 color);
void draw_model(Model model, Vector3 position, Vector3 scale, Quaternion orientation, Vector4 color, Render_Options options, bool draw_transparency);
void draw_texture(Texture texture, Vector3 min, Vector3 max, float z_override = 0);

struct Fixed_Function {
    Array<Vertex> *array;
    Buffer vertex_buffer;
    Vertex *current_vertex;
};

void ff_begin(Fixed_Function *ff, Array<Vertex> *array);
void ff_flush(Fixed_Function *ff);
void ff_end(Fixed_Function *ff);

void ff_vertex(Fixed_Function *ff, Vector3 position);
void ff_tex_coord(Fixed_Function *ff, Vector3 tex_coord);
void ff_color(Fixed_Function *ff, Vector4 color);
void ff_quad(Fixed_Function *ff, Vector3 min, Vector3 max, Vector4 color, Vector3 uv_overrides[2] = nullptr);
void ff_line(Fixed_Function *ff, Vector3 a, Vector3 b, Vector4 color);
void ff_line_circle(Fixed_Function *ff, Vector3 position, float radius, Vector3 up, Vector4 color, float resolution = 2);
void ff_text(Fixed_Function *ff, char *str, Font font, Vector4 color, Vector3 start_pos, float size);
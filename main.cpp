#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define DEVELOPER

#include "window.h"
#include "basic.h"
#include "math.h"
#include "render_backend.h"
#include "renderer.h"
#include "array.h"

#ifdef DEVELOPER
#include "assimp_loader.cpp"
#endif

/*
TODO:
-text
-directional lights
-point lights
-spot lights
-shadow maps
-hdr
-bloom
-skybox
-AO?
-instancing
-anti aliasing
-transparency handling
*/

static Window g_main_window;
static float g_time_at_startup;

void main() {
    init_platform();

    g_time_at_startup = time_now();

    Allocator global_allocator = default_allocator();

    g_main_window = create_window(1920, 1080);

    init_render_backend(&g_main_window);
    init_renderer(&g_main_window);

    Vertex_Shader vertex_shader = compile_vertex_shader_from_file(L"vertex.hlsl");
    Pixel_Shader pixel_shader = compile_pixel_shader_from_file(L"pixel.hlsl");

    // Make vertex format
    Vertex_Field vertex_fields[] = {
        {"SV_POSITION", "position",  offsetof(Vertex, position),  VFT_FLOAT3, VFST_PER_VERTEX},
        {"TEXCOORD",    "tex_coord", offsetof(Vertex, tex_coord), VFT_FLOAT3, VFST_PER_VERTEX},
        {"COLOR",       "color",     offsetof(Vertex, color),     VFT_FLOAT4, VFST_PER_VERTEX},
        {"NORMAL",      "normal",    offsetof(Vertex, normal),    VFT_FLOAT3, VFST_PER_VERTEX},
        {"TANGENT",     "tangent",   offsetof(Vertex, tangent),   VFT_FLOAT3, VFST_PER_VERTEX},
        {"BITANGENT",   "bitangent", offsetof(Vertex, bitangent), VFT_FLOAT3, VFST_PER_VERTEX},
    };
    Vertex_Format default_vertex_format = create_vertex_format(vertex_fields, ARRAYSIZE(vertex_fields), vertex_shader);

    u32 cube_indices[] = {
         0,  2,  1,  0,  3,  2,
         4,  5,  6,  4,  6,  7,
         8, 10,  9,  8, 11, 10,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 22, 21, 20, 23, 22,
    };

    Vertex cube_vertices[] = {
        {{-(0.5f), -(0.5f), -(0.5f)}, {1, 0, 0}, {1, 1, 1, 1}},
        {{ (0.5f), -(0.5f), -(0.5f)}, {0, 0, 0}, {1, 1, 1, 1}},
        {{ (0.5f),  (0.5f), -(0.5f)}, {0, 1, 0}, {1, 1, 1, 1}},
        {{-(0.5f),  (0.5f), -(0.5f)}, {1, 1, 0}, {1, 1, 1, 1}},

        {{-(0.5f), -(0.5f),  (0.5f)}, {0, 0, 0}, {1, 1, 1, 1}},
        {{ (0.5f), -(0.5f),  (0.5f)}, {1, 0, 0}, {1, 1, 1, 1}},
        {{ (0.5f),  (0.5f),  (0.5f)}, {1, 1, 0}, {1, 1, 1, 1}},
        {{-(0.5f),  (0.5f),  (0.5f)}, {0, 1, 0}, {1, 1, 1, 1}},

        {{-(0.5f), -(0.5f), -(0.5f)}, {0, 0, 0}, {1, 1, 1, 1}},
        {{-(0.5f),  (0.5f), -(0.5f)}, {0, 1, 0}, {1, 1, 1, 1}},
        {{-(0.5f),  (0.5f),  (0.5f)}, {1, 1, 0}, {1, 1, 1, 1}},
        {{-(0.5f), -(0.5f),  (0.5f)}, {1, 0, 0}, {1, 1, 1, 1}},

        {{ (0.5f), -(0.5f), -(0.5f)}, {1, 0, 0}, {1, 1, 1, 1}},
        {{ (0.5f),  (0.5f), -(0.5f)}, {1, 1, 0}, {1, 1, 1, 1}},
        {{ (0.5f),  (0.5f),  (0.5f)}, {0, 1, 0}, {1, 1, 1, 1}},
        {{ (0.5f), -(0.5f),  (0.5f)}, {0, 0, 0}, {1, 1, 1, 1}},

        {{-(0.5f), -(0.5f), -(0.5f)}, {0, 0, 0}, {1, 1, 1, 1}},
        {{ (0.5f), -(0.5f), -(0.5f)}, {1, 0, 0}, {1, 1, 1, 1}},
        {{ (0.5f), -(0.5f),  (0.5f)}, {1, 1, 0}, {1, 1, 1, 1}},
        {{-(0.5f), -(0.5f),  (0.5f)}, {0, 1, 0}, {1, 1, 1, 1}},

        {{-(0.5f),  (0.5f), -(0.5f)}, {0, 1, 0}, {1, 1, 1, 1}},
        {{ (0.5f),  (0.5f), -(0.5f)}, {1, 1, 0}, {1, 1, 1, 1}},
        {{ (0.5f),  (0.5f),  (0.5f)}, {1, 0, 0}, {1, 1, 1, 1}},
        {{-(0.5f),  (0.5f),  (0.5f)}, {0, 0, 0}, {1, 1, 1, 1}},
    };

    Buffer cube_vertex_buffer = create_buffer(BT_VERTEX, cube_vertices, sizeof(cube_vertices));
    Buffer cube_index_buffer  = create_buffer(BT_INDEX,  cube_indices,  sizeof(cube_indices));

    Vector3 camera_position = {};
    Quaternion camera_orientation = quaternion_identity();

    Array<Loaded_Mesh> helmet_meshes = {};
    helmet_meshes.allocator = default_allocator();
    load_mesh_from_file("sponza/DamagedHelmet.gltf", &helmet_meshes);

    Array<Loaded_Mesh> sponza_meshes = {};
    sponza_meshes.allocator = default_allocator();
    load_mesh_from_file("sponza/sponza.glb", &sponza_meshes);

    Render_Options render_options = {};
    render_options.do_albedo    = true;
    render_options.do_normal    = true;
    render_options.do_metallic  = true;
    render_options.do_roughness = true;
    render_options.do_emission  = true;
    render_options.do_ao        = true;

    Buffer lighting_cbuffer_handle = create_buffer(BT_CONSTANT, nullptr, sizeof(Lighting_CBuffer));

    Quaternion helmet_orientation = quaternion_identity();

    float time_since_startup = 0;
    const double time_at_startup = time_now();
    double last_frame_start_time = time_now();
    while (true) {
        double this_frame_start_time = time_now();
        time_since_startup = (float)(this_frame_start_time - time_at_startup);
        double true_delta_time = this_frame_start_time - last_frame_start_time;
        defer(last_frame_start_time = this_frame_start_time);

        update_window(&g_main_window);
        if (g_main_window.should_close) {
            break;
        }

        if (get_input(&g_main_window, INPUT_ESCAPE)) {
            break;
        }

        if (get_input_down(&g_main_window, INPUT_1)) { render_options.do_albedo         = !render_options.do_albedo;         printf("do_albedo: %d\n",         render_options.do_albedo);         }
        if (get_input_down(&g_main_window, INPUT_2)) { render_options.do_normal         = !render_options.do_normal;         printf("do_normal: %d\n",         render_options.do_normal);         }
        if (get_input_down(&g_main_window, INPUT_3)) { render_options.do_metallic       = !render_options.do_metallic;       printf("do_metallic: %d\n",       render_options.do_metallic);       }
        if (get_input_down(&g_main_window, INPUT_4)) { render_options.do_roughness      = !render_options.do_roughness;      printf("do_roughness: %d\n",      render_options.do_roughness);      }
        if (get_input_down(&g_main_window, INPUT_5)) { render_options.do_emission       = !render_options.do_emission;       printf("do_emission: %d\n",       render_options.do_emission);       }
        if (get_input_down(&g_main_window, INPUT_6)) { render_options.do_ao             = !render_options.do_ao;             printf("do_ao: %d\n",             render_options.do_ao);             }
        if (get_input_down(&g_main_window, INPUT_7)) { render_options.visualize_normals = !render_options.visualize_normals; printf("visualize_normals: %d\n", render_options.visualize_normals); }

        const float CAMERA_SPEED = 0.025f;

        if (get_input(&g_main_window, INPUT_E)) camera_position += quaternion_up(camera_orientation)      * CAMERA_SPEED;
        if (get_input(&g_main_window, INPUT_Q)) camera_position -= quaternion_up(camera_orientation)      * CAMERA_SPEED;
        if (get_input(&g_main_window, INPUT_W)) camera_position += quaternion_forward(camera_orientation) * CAMERA_SPEED;
        if (get_input(&g_main_window, INPUT_S)) camera_position -= quaternion_forward(camera_orientation) * CAMERA_SPEED;
        if (get_input(&g_main_window, INPUT_D)) camera_position += quaternion_right(camera_orientation)   * CAMERA_SPEED;
        if (get_input(&g_main_window, INPUT_A)) camera_position -= quaternion_right(camera_orientation)   * CAMERA_SPEED;

        if (get_input(&g_main_window, INPUT_MOUSE_RIGHT)) {
            Vector2 delta = g_main_window.mouse_position_pixel_delta * 0.25f;
            Vector3 rotate_vector = v3(-delta.y, delta.x, 0);

            Quaternion x = axis_angle(v3(1, 0, 0), to_radians(rotate_vector.x));
            Quaternion y = axis_angle(v3(0, 1, 0), to_radians(rotate_vector.y));
            Quaternion z = axis_angle(v3(0, 0, 1), to_radians(rotate_vector.z));
            Quaternion result = y * camera_orientation;
            result = result * x;
            result = result * z;
            result = normalize(result);
            camera_orientation = result;
        }

        prerender(g_main_window.width, g_main_window.height);

        set_render_targets(nullptr, nullptr);
        clear_bound_render_targets(v4(0.39, 0.58, 0.93, 1.0f));

        bind_vertex_format(default_vertex_format);
        bind_shaders(vertex_shader, pixel_shader);

        Lighting_CBuffer lighting = {};
        lighting.point_light_positions[lighting.num_point_lights] = v4(sin(time_since_startup) * 3, 6, 0, 1);
        lighting.point_light_colors[lighting.num_point_lights++]  = v4(1, 0, 0, 1) * 50;
        lighting.point_light_positions[lighting.num_point_lights] = v4(sin(time_since_startup * 0.6) * 3, 6, 0, 1);
        lighting.point_light_colors[lighting.num_point_lights++]  = v4(0, 1, 0, 1) * 50;
        lighting.point_light_positions[lighting.num_point_lights] = v4(sin(time_since_startup * 0.7) * 3, 6, 0, 1);
        lighting.point_light_colors[lighting.num_point_lights++]  = v4(0, 0, 1, 1) * 50;
        update_buffer(lighting_cbuffer_handle, &lighting, sizeof(Lighting_CBuffer));
        bind_constant_buffers(&lighting_cbuffer_handle, 1, CBS_LIGHTING);

        Render_Pass_Desc pass = {};
        pass.camera_position = camera_position;
        pass.camera_orientation = camera_orientation;
        pass.projection_matrix = perspective(to_radians(60), (float)g_main_window.width / (float)g_main_window.height, 0.001, 1000);
        begin_render_pass(&pass);

        helmet_orientation = axis_angle(v3(0, 1, 0), time_since_startup * 0.5);

        draw_meshes(sponza_meshes, v3(0, 0, 0), v3(1, 1, 1), quaternion_identity(), render_options);
        draw_meshes(helmet_meshes, v3(0, 4, 0), v3(1, 1, 1), helmet_orientation, render_options);

        present(true);
    }
}
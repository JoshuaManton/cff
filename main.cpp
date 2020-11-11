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
-draw commands
-depth sorting
-transparency sorting
-spot lights
-cascaded shadow maps
-bloom
-skybox
-AO?
-instancing (do we support this already?)
-anti aliasing
*/

static Window g_main_window;
static float g_time_at_startup;

void draw_texture(Texture texture, Vector3 min, Vector3 max, Vertex_Shader vertex_shader, Pixel_Shader pixel_shader) {
    Render_Pass_Desc pass = {};
    pass.camera_position = v3(0, 0, 0);
    pass.camera_orientation = quaternion_identity();
    pass.projection_matrix = orthographic(0, g_main_window.width, 0, g_main_window.height, -1, 1);
    begin_render_pass(&pass);
    Vertex ffverts[6];
    Fixed_Function ff = {};
    ff_begin(&ff, ffverts, ARRAYSIZE(ffverts), texture, vertex_shader, pixel_shader);
    Vector2 uvs[2] = {
        v2(0, 1),
        v2(1, 0),
    };
    ff_quad(&ff, min, max, v4(1, 1, 1, 1), uvs);
    ff_end(&ff);
}

void draw_scene(Render_Options render_options, float time_since_startup, Array<Loaded_Mesh> sponza_meshes, Array<Loaded_Mesh> helmet_meshes) {
    Quaternion helmet_orientation = axis_angle(v3(0, 1, 0), time_since_startup * 0.5);
    draw_meshes(sponza_meshes, v3(0, 0, 0), v3(1, 1, 1), quaternion_identity(), render_options, false);
    draw_meshes(helmet_meshes, v3(0, 4, 0), v3(1, 1, 1), helmet_orientation, render_options, false);
    draw_meshes(sponza_meshes, v3(0, 0, 0), v3(1, 1, 1), quaternion_identity(), render_options, true);
    draw_meshes(helmet_meshes, v3(0, 4, 0), v3(1, 1, 1), helmet_orientation, render_options, true);
}

void main() {
    init_platform();

    g_time_at_startup = time_now();

    Allocator global_allocator = default_allocator();

    g_main_window = create_window(1920, 1080);

    init_render_backend(&g_main_window);
    init_renderer(&g_main_window);

    Vertex_Shader vertex_shader       = compile_vertex_shader_from_file(L"vertex.hlsl");
    Pixel_Shader  pixel_shader        = compile_pixel_shader_from_file(L"pixel.hlsl");
    Pixel_Shader  simple_pixel_shader = compile_pixel_shader_from_file(L"ff_pixel.hlsl");
    Pixel_Shader  text_pixel_shader   = compile_pixel_shader_from_file(L"text_pixel.hlsl");
    Pixel_Shader  shadow_pixel_shader = compile_pixel_shader_from_file(L"shadow_pixel.hlsl");

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

    Font roboto_mono = load_font_from_file("fonts/roboto_mono.ttf", 32);
    Font roboto      = load_font_from_file("fonts/roboto.ttf", 32);

    Array<Loaded_Mesh> helmet_meshes = {};
    helmet_meshes.allocator = default_allocator();
    load_mesh_from_file("sponza/DamagedHelmet.gltf", &helmet_meshes);

    Array<Loaded_Mesh> sponza_meshes = {};
    sponza_meshes.allocator = default_allocator();
    load_mesh_from_file("sponza/sponza.glb", &sponza_meshes);

    Render_Options render_options = {};
    render_options.do_albedo_map    = true;
    render_options.do_normal_map    = true;
    render_options.do_metallic_map  = true;
    render_options.do_roughness_map = true;
    render_options.do_emission_map  = true;
    render_options.do_ao_map        = true;

    Texture shadow_map_color_buffer = {};
    Texture shadow_map_depth_buffer = {};
    create_color_and_depth_buffers(2048, 2048, TF_R16G16B16A16_FLOAT, &shadow_map_color_buffer, &shadow_map_depth_buffer);

    Texture hdr_color_buffer = {};
    Texture hdr_depth_buffer = {};
    create_color_and_depth_buffers(g_main_window.width, g_main_window.height, TF_R16G16B16A16_FLOAT, &hdr_color_buffer, &hdr_depth_buffer);

    Buffer lighting_cbuffer_handle = create_buffer(BT_CONSTANT, nullptr, sizeof(Lighting_CBuffer));

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

        if (get_input_down(&g_main_window, INPUT_1)) { render_options.do_albedo_map     = !render_options.do_albedo_map;     }
        if (get_input_down(&g_main_window, INPUT_2)) { render_options.do_normal_map     = !render_options.do_normal_map;     }
        if (get_input_down(&g_main_window, INPUT_3)) { render_options.do_metallic_map   = !render_options.do_metallic_map;   }
        if (get_input_down(&g_main_window, INPUT_4)) { render_options.do_roughness_map  = !render_options.do_roughness_map;  }
        if (get_input_down(&g_main_window, INPUT_5)) { render_options.do_emission_map   = !render_options.do_emission_map;   }
        if (get_input_down(&g_main_window, INPUT_6)) { render_options.do_ao_map         = !render_options.do_ao_map;         }
        if (get_input_down(&g_main_window, INPUT_7)) { render_options.visualize_normals = !render_options.visualize_normals; }

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

        prerender();

        set_render_targets(nullptr, nullptr);
        clear_bound_render_targets(v4(0.39, 0.58, 0.93, 1.0f));

        bind_vertex_format(default_vertex_format);

        Matrix4 sun_transform = {};

        Quaternion sun_orientation = axis_angle(v3(0, 1, 0), to_radians(90 + sin(time_since_startup * 0.04) * 30)) * axis_angle(v3(1, 0, 0), to_radians(90 + sin(time_since_startup * 0.043) * 30));

        // draw scene to shadow map
        {
            Texture *color_buffers[MAX_COLOR_BUFFERS] = {
                &shadow_map_color_buffer,
            };
            set_render_targets(color_buffers, &shadow_map_depth_buffer);
            clear_bound_render_targets(v4(0, 0, 0, 0));

            Render_Pass_Desc scene_pass = {};
            scene_pass.camera_position = v3(0, 20, 0);
            scene_pass.camera_orientation = sun_orientation;
            scene_pass.projection_matrix = orthographic(-20, 20, -20, 20, -100, 100);
            sun_transform = scene_pass.projection_matrix * view_matrix(scene_pass.camera_position, scene_pass.camera_orientation);
            begin_render_pass(&scene_pass);
            bind_shaders(vertex_shader, shadow_pixel_shader);
            draw_scene(render_options, time_since_startup, sponza_meshes, helmet_meshes);
        }

        Lighting_CBuffer lighting = {};
        lighting.point_light_positions[lighting.num_point_lights] = v4(sin(time_since_startup) * 3, 6, 0, 1);
        lighting.point_light_colors[lighting.num_point_lights++]  = v4(1, 0, 0, 1) * 400;
        lighting.point_light_positions[lighting.num_point_lights] = v4(sin(time_since_startup * 0.6) * 3, 6, 0, 1);
        lighting.point_light_colors[lighting.num_point_lights++]  = v4(0, 1, 0, 1) * 400;
        lighting.point_light_positions[lighting.num_point_lights] = v4(sin(time_since_startup * 0.7) * 3, 6, 0, 1);
        lighting.point_light_colors[lighting.num_point_lights++]  = v4(0, 0, 1, 1) * 400;
        lighting.sun_direction = quaternion_forward(sun_orientation);
        lighting.sun_color = v3(1, 1, 1) * 100;
        lighting.sun_transform = sun_transform;
        update_buffer(lighting_cbuffer_handle, &lighting, sizeof(Lighting_CBuffer));
        bind_constant_buffers(&lighting_cbuffer_handle, 1, CBS_LIGHTING);

        // draw scene to hdr buffer
        {
            Texture *color_buffers[MAX_COLOR_BUFFERS] = {
                &hdr_color_buffer,
            };
            set_render_targets(color_buffers, &hdr_depth_buffer);
            clear_bound_render_targets(v4(0.39, 0.58, 0.93, 1.0f));

            bind_textures(&shadow_map_color_buffer, 1, TS_SHADOW_MAP);

            Render_Pass_Desc scene_pass = {};
            scene_pass.camera_position = camera_position;
            scene_pass.camera_orientation = camera_orientation;
            scene_pass.projection_matrix = perspective(to_radians(60), (float)g_main_window.width / (float)g_main_window.height, 0.001, 1000);
            begin_render_pass(&scene_pass);
            bind_shaders(vertex_shader, pixel_shader);
            draw_scene(render_options, time_since_startup, sponza_meshes, helmet_meshes);
        }

        set_render_targets(nullptr, nullptr);

        draw_texture(hdr_color_buffer, v3(0, 0, 0), v3(g_main_window.width, g_main_window.height, 0), vertex_shader, simple_pixel_shader);
        // draw_texture(shadow_map_color_buffer, v3(0, 0, 0), v3(256, 256, 0), vertex_shader, simple_pixel_shader);

        Render_Pass_Desc ui_pass = {};
        ui_pass.camera_position = v3(0, 0, 0);
        ui_pass.camera_orientation = quaternion_identity();
        ui_pass.projection_matrix = orthographic(0, g_main_window.width, 0, g_main_window.height, -1, 1);
        begin_render_pass(&ui_pass);
        Vertex ffverts[1024];
        Fixed_Function ff = {};
        ff_begin(&ff, ffverts, ARRAYSIZE(ffverts), roboto_mono.texture, vertex_shader, text_pixel_shader);

        Vector3 text_pos = v3(10, g_main_window.height-roboto_mono.pixel_height, 0);
        const float text_size = 1;
        ff_text(&ff, "1. do_albedo_map",     roboto_mono, v4(1, 1, 1, render_options.do_albedo_map     ? 1.0 : 0.2), text_pos, text_size); text_pos.y -= roboto_mono.pixel_height * text_size;
        ff_text(&ff, "2. do_normal_map",     roboto_mono, v4(1, 1, 1, render_options.do_normal_map     ? 1.0 : 0.2), text_pos, text_size); text_pos.y -= roboto_mono.pixel_height * text_size;
        ff_text(&ff, "3. do_metallic_map",   roboto_mono, v4(1, 1, 1, render_options.do_metallic_map   ? 1.0 : 0.2), text_pos, text_size); text_pos.y -= roboto_mono.pixel_height * text_size;
        ff_text(&ff, "4. do_roughness_map",  roboto_mono, v4(1, 1, 1, render_options.do_roughness_map  ? 1.0 : 0.2), text_pos, text_size); text_pos.y -= roboto_mono.pixel_height * text_size;
        ff_text(&ff, "5. do_emission_map",   roboto_mono, v4(1, 1, 1, render_options.do_emission_map   ? 1.0 : 0.2), text_pos, text_size); text_pos.y -= roboto_mono.pixel_height * text_size;
        ff_text(&ff, "6. do_ao_map",         roboto_mono, v4(1, 1, 1, render_options.do_ao_map         ? 1.0 : 0.2), text_pos, text_size); text_pos.y -= roboto_mono.pixel_height * text_size;
        ff_text(&ff, "7. visualize_normals", roboto_mono, v4(1, 1, 1, render_options.visualize_normals ? 1.0 : 0.2), text_pos, text_size); text_pos.y -= roboto_mono.pixel_height * text_size;
        ff_end(&ff);

        present(true);
    }
}
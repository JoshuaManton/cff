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
-cubemaps
-skybox
-instancing (do we support this already?)
-spot lights
-fix alpha blending without ruining bloom
-cascaded shadow maps
-draw commands
-depth sorting to reduce overdraw
-transparency sorting to alpha blend properly
-particle systems?
-AO?
*/

void draw_texture(Texture texture, Vector3 min, Vector3 max, float z_override = 0) {
    Vertex ffverts[6];
    Fixed_Function ff = {};
    ff_begin(&ff, ffverts, ARRAYSIZE(ffverts));
    bind_texture(texture, 0);
    Vector3 uvs[2] = {
        v3(0, 1, z_override),
        v3(1, 0, z_override),
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

struct Blur_CBuffer {
    int horizontal;
    Vector2 buffer_dimensions;
    float _pad;
};

#define BLOOM_BUFFER_DOWNSCALE 8.0

struct Compute_VP_CBuffer {
    Matrix4 view_matrix;
    Matrix4 projection_matrix;
};

void main() {
    init_platform();

    Allocator global_allocator = default_allocator();

    Window main_window = create_window(1920, 1080);

    init_render_backend(&main_window);
    init_renderer(&main_window);

    Compute_Shader test_compute_shader = compile_compute_shader_from_file(L"test_compute.hlsl");

    Texture_Description test_3d_texture_description = {};
    test_3d_texture_description.width  = 128;
    test_3d_texture_description.height = 128;
    test_3d_texture_description.depth  = 128;
    test_3d_texture_description.uav = true;
    test_3d_texture_description.type = TT_3D;
    test_3d_texture_description.format = TF_R32G32B32A32_FLOAT;
    Texture test_3d_texture = create_texture(test_3d_texture_description);

    Vertex_Shader vertex_shader                = compile_vertex_shader_from_file(L"vertex.hlsl");
    Pixel_Shader  pixel_shader                 = compile_pixel_shader_from_file(L"pixel.hlsl");
    Pixel_Shader  simple_pixel_shader          = compile_pixel_shader_from_file(L"simple_pixel.hlsl");
    Pixel_Shader  simple_pixel_textured_shader = compile_pixel_shader_from_file(L"simple_pixel_textured.hlsl");
    Pixel_Shader  simple_pixel_3d_shader       = compile_pixel_shader_from_file(L"simple_pixel_3d.hlsl");
    Pixel_Shader  text_pixel_shader            = compile_pixel_shader_from_file(L"text_pixel.hlsl");
    Pixel_Shader  shadow_pixel_shader          = compile_pixel_shader_from_file(L"shadow_pixel.hlsl");
    Pixel_Shader  blur_pixel_shader            = compile_pixel_shader_from_file(L"blur_pixel.hlsl");
    Pixel_Shader  final_pixel_shader           = compile_pixel_shader_from_file(L"final_pixel.hlsl");

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

        {{-(0.5f), -(0.5f), -(0.5f)}, {0, 0 ,0}, {1, 1, 1, 1}},
        {{ (0.5f), -(0.5f), -(0.5f)}, {1, 0, 0}, {1, 1, 1, 1}},
        {{ (0.5f), -(0.5f),  (0.5f)}, {1, 1, 0}, {1, 1, 1, 1}},
        {{-(0.5f), -(0.5f),  (0.5f)}, {0, 1, 0}, {1, 1, 1, 1}},

        {{-(0.5f),  (0.5f), -(0.5f)}, {0, 1, 0}, {1, 1, 1, 1}},
        {{ (0.5f),  (0.5f), -(0.5f)}, {1, 1, 0}, {1, 1, 1, 1}},
        {{ (0.5f),  (0.5f),  (0.5f)}, {1, 0, 0}, {1, 1, 1, 1}},
        {{-(0.5f),  (0.5f),  (0.5f)}, {0, 0, 0}, {1, 1, 1, 1}},
    };

    Font roboto_mono = load_font_from_file("fonts/roboto_mono.ttf", 32);
    Font roboto      = load_font_from_file("fonts/roboto.ttf", 32);

    Render_Options render_options = {};
    render_options.do_albedo_map    = true;
    render_options.do_normal_map    = true;
    render_options.do_metallic_map  = true;
    render_options.do_roughness_map = true;
    render_options.do_emission_map  = true;
    render_options.do_ao_map        = true;

    Vector3 camera_position = {};
    Quaternion camera_orientation = quaternion_identity();

    Array<Loaded_Mesh> helmet_meshes = {};
    helmet_meshes.allocator = default_allocator();
    load_mesh_from_file("sponza/DamagedHelmet.gltf", &helmet_meshes);

    Array<Loaded_Mesh> sponza_meshes = {};
    sponza_meshes.allocator = default_allocator();
    load_mesh_from_file("sponza/sponza.glb", &sponza_meshes);

    Texture shadow_map_color_buffer = {};
    Texture shadow_map_depth_buffer = {};
    Texture_Description shadow_map_description = {};
    shadow_map_description.width = 2048;
    shadow_map_description.height = 2048;
    shadow_map_description.format = TF_R16G16B16A16_FLOAT;
    shadow_map_description.wrap_mode = TWM_POINT_CLAMP;
    create_color_and_depth_buffers(shadow_map_description, &shadow_map_color_buffer, &shadow_map_depth_buffer);

    Texture hdr_color_buffer = {};
    Texture hdr_depth_buffer = {};
    Texture_Description hdr_description = {};
    hdr_description.width = main_window.width;
    hdr_description.height = main_window.height;
    hdr_description.format = TF_R16G16B16A16_FLOAT;
    hdr_description.wrap_mode = TWM_LINEAR_CLAMP;
    hdr_description.sample_count = 8;
    create_color_and_depth_buffers(hdr_description, &hdr_color_buffer, &hdr_depth_buffer);

    Texture_Description bloom_desc = {};
    bloom_desc.width  = main_window.width;
    bloom_desc.height = main_window.height;
    bloom_desc.type = TT_2D;
    bloom_desc.format = TF_R16G16B16A16_FLOAT;
    bloom_desc.wrap_mode = TWM_LINEAR_CLAMP;
    bloom_desc.render_target = true;
    bloom_desc.sample_count = 8;
    Texture bloom_color_buffer = create_texture(bloom_desc);

    Texture_Description bloom_ping_pong_desc = {};
    bloom_ping_pong_desc.width  = main_window.width  / BLOOM_BUFFER_DOWNSCALE;
    bloom_ping_pong_desc.height = main_window.height / BLOOM_BUFFER_DOWNSCALE;
    bloom_ping_pong_desc.type = TT_2D;
    bloom_ping_pong_desc.wrap_mode = TWM_LINEAR_CLAMP;
    bloom_ping_pong_desc.format = TF_R16G16B16A16_FLOAT;
    bloom_ping_pong_desc.render_target = true;
    Texture bloom_ping_pong_color_buffers[2] = {};
    bloom_ping_pong_color_buffers[0] = create_texture(bloom_ping_pong_desc);
    bloom_ping_pong_color_buffers[1] = create_texture(bloom_ping_pong_desc);
    Texture_Description bloom_ping_pong_depth_desc = bloom_ping_pong_desc;
    bloom_ping_pong_depth_desc.format = TF_DEPTH_STENCIL;
    Texture bloom_ping_pong_depth_buffer = create_texture(bloom_ping_pong_depth_desc);

    Buffer lighting_cbuffer_handle = create_buffer(BT_CONSTANT, nullptr, sizeof(Lighting_CBuffer));
    Buffer blur_cbuffer_handle     = create_buffer(BT_CONSTANT, nullptr, sizeof(Blur_CBuffer));
    Buffer compute_vp_cbuffer      = create_buffer(BT_CONSTANT, nullptr, sizeof(Compute_VP_CBuffer));

    const float FIXED_DT = 1.0f / 120;

    float time_since_startup = 0;
    const double time_at_startup = time_now();
    double last_frame_start_time = time_now();
    float dt_acc = 0;
    while (!main_window.should_close) {
        double this_frame_start_time = time_now();
        time_since_startup = (float)(this_frame_start_time - time_at_startup);
        defer(last_frame_start_time = this_frame_start_time);
        double true_delta_time = this_frame_start_time - last_frame_start_time;
        true_delta_time = min(0.2, true_delta_time); // note(josh): stop spiral of death
        dt_acc += true_delta_time;
        while (dt_acc >= FIXED_DT) {
            dt_acc -= FIXED_DT;
            float dt = FIXED_DT;

            update_window(&main_window);
            if (get_input(&main_window, INPUT_ESCAPE)) {
                main_window.should_close = true;
                break;
            }

            if (get_input_down(&main_window, INPUT_1)) { render_options.do_albedo_map     = !render_options.do_albedo_map;     }
            if (get_input_down(&main_window, INPUT_2)) { render_options.do_normal_map     = !render_options.do_normal_map;     }
            if (get_input_down(&main_window, INPUT_3)) { render_options.do_metallic_map   = !render_options.do_metallic_map;   }
            if (get_input_down(&main_window, INPUT_4)) { render_options.do_roughness_map  = !render_options.do_roughness_map;  }
            if (get_input_down(&main_window, INPUT_5)) { render_options.do_emission_map   = !render_options.do_emission_map;   }
            if (get_input_down(&main_window, INPUT_6)) { render_options.do_ao_map         = !render_options.do_ao_map;         }
            if (get_input_down(&main_window, INPUT_7)) { render_options.visualize_normals = !render_options.visualize_normals; }

            // camera movement
            {
                const float CAMERA_SPEED_BASE = 5;
                const float CAMERA_SPEED_FAST = 20;
                const float CAMERA_SPEED_SLOW = 0.5;

                float camera_speed = CAMERA_SPEED_BASE;
                     if (get_input(&main_window, INPUT_SHIFT)) camera_speed = CAMERA_SPEED_FAST;
                else if (get_input(&main_window, INPUT_ALT))   camera_speed = CAMERA_SPEED_SLOW;

                if (get_input(&main_window, INPUT_E)) camera_position += quaternion_up(camera_orientation)      * camera_speed * dt;
                if (get_input(&main_window, INPUT_Q)) camera_position -= quaternion_up(camera_orientation)      * camera_speed * dt;
                if (get_input(&main_window, INPUT_W)) camera_position += quaternion_forward(camera_orientation) * camera_speed * dt;
                if (get_input(&main_window, INPUT_S)) camera_position -= quaternion_forward(camera_orientation) * camera_speed * dt;
                if (get_input(&main_window, INPUT_D)) camera_position += quaternion_right(camera_orientation)   * camera_speed * dt;
                if (get_input(&main_window, INPUT_A)) camera_position -= quaternion_right(camera_orientation)   * camera_speed * dt;

                if (get_input(&main_window, INPUT_MOUSE_RIGHT)) {
                    Vector2 delta = main_window.mouse_position_pixel_delta * 0.25f;
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
            }
        }

        prerender();

        ensure_swap_chain_size(main_window.width, main_window.height);

        ensure_texture_size(&hdr_color_buffer, main_window.width, main_window.height);
        ensure_texture_size(&hdr_depth_buffer, main_window.width, main_window.height);
        ensure_texture_size(&bloom_color_buffer, main_window.width, main_window.height);
        for (int i = 0; i < ARRAYSIZE(bloom_ping_pong_color_buffers); i++) {
            ensure_texture_size(&bloom_ping_pong_color_buffers[i], main_window.width / BLOOM_BUFFER_DOWNSCALE, main_window.height / BLOOM_BUFFER_DOWNSCALE);
        }
        ensure_texture_size(&bloom_ping_pong_depth_buffer, main_window.width / BLOOM_BUFFER_DOWNSCALE, main_window.height / BLOOM_BUFFER_DOWNSCALE);

        bind_vertex_format(default_vertex_format);

        Matrix4 sun_transform = {};

        Quaternion sun_orientation = axis_angle(v3(0, 1, 0), to_radians(90 + sin(time_since_startup * 0.04) * 30)) * axis_angle(v3(1, 0, 0), to_radians(90 + sin(time_since_startup * 0.043) * 30));

        // draw scene to shadow map
        {
            set_render_targets(&shadow_map_color_buffer, 1, &shadow_map_depth_buffer);
            clear_bound_render_targets(v4(0, 0, 0, 0));

            Render_Pass_Desc shadow_pass = {};
            shadow_pass.camera_position = v3(0, 20, 0);
            shadow_pass.camera_orientation = sun_orientation;
            shadow_pass.projection_matrix = construct_orthographic_matrix(-20, 20, -20, 20, -100, 100);
            sun_transform = shadow_pass.projection_matrix * construct_view_matrix(shadow_pass.camera_position, shadow_pass.camera_orientation);
            begin_render_pass(&shadow_pass);
            bind_shaders(vertex_shader, shadow_pixel_shader);
            draw_scene(render_options, time_since_startup, sponza_meshes, helmet_meshes);
            end_render_pass();
            unset_render_targets();
        }

        Lighting_CBuffer lighting = {};
        // lighting.point_light_positions[lighting.num_point_lights] = v4(sin(time_since_startup) * 3, 6, 0, 1);
        // lighting.point_light_colors[lighting.num_point_lights++]  = v4(1, 0, 0, 1) * 500;
        // lighting.point_light_positions[lighting.num_point_lights] = v4(sin(time_since_startup * 0.6) * 3, 6, 0, 1);
        // lighting.point_light_colors[lighting.num_point_lights++]  = v4(0, 1, 0, 1) * 500;
        // lighting.point_light_positions[lighting.num_point_lights] = v4(sin(time_since_startup * 0.7) * 3, 6, 0, 1);
        // lighting.point_light_colors[lighting.num_point_lights++]  = v4(0, 0, 1, 1) * 500;
        lighting.sun_direction = quaternion_forward(sun_orientation);
        lighting.sun_color = v3(1, 1, 1) * 200;
        lighting.sun_transform = sun_transform;
        lighting.fog_base_color = v3(1, 1, 1);
        lighting.fog_density    = 0.05;
        lighting.fog_y_level    = -1;
        update_buffer(lighting_cbuffer_handle, &lighting, sizeof(Lighting_CBuffer));
        bind_constant_buffers(&lighting_cbuffer_handle, 1, CBS_LIGHTING);

        Compute_VP_CBuffer vp_cbuffer = {};
        vp_cbuffer.view_matrix = construct_trs_matrix(camera_position, camera_orientation, v3(1, 1, 1));
        vp_cbuffer.projection_matrix = construct_perspective_matrix(to_radians(60), (float)main_window.width / (float)main_window.height, 0.001, 1000);
        update_buffer(compute_vp_cbuffer, &vp_cbuffer, sizeof(Compute_VP_CBuffer));
        bind_constant_buffers(&compute_vp_cbuffer, 1, 0);
        bind_compute_shader(test_compute_shader);
        bind_compute_uav(test_3d_texture, 0);
        dispatch_compute(test_3d_texture.description.width, test_3d_texture.description.height, test_3d_texture.description.depth);
        bind_compute_uav({}, 0);

        // draw scene to hdr buffer
        {
            Texture color_buffers[2] = {
                hdr_color_buffer,
                bloom_color_buffer,
            };
            set_render_targets(color_buffers, ARRAYSIZE(color_buffers), &hdr_depth_buffer);
            clear_bound_render_targets(v4(0, 0, 0, 0));

            bind_texture(shadow_map_color_buffer, TS_PBR_SHADOW_MAP);
            bind_texture(test_3d_texture, TS_PBR_CAMERA_BOX);

            Render_Pass_Desc scene_pass = {};
            scene_pass.camera_position = camera_position;
            scene_pass.camera_orientation = camera_orientation;
            scene_pass.projection_matrix = construct_perspective_matrix(to_radians(60), (float)main_window.width / (float)main_window.height, 0.001, 1000);

            begin_render_pass(&scene_pass);
            bind_shaders(vertex_shader, pixel_shader);
            draw_scene(render_options, time_since_startup, sponza_meshes, helmet_meshes);
            end_render_pass();
            unset_render_targets();
            bind_texture({}, TS_PBR_SHADOW_MAP);
        }

        Texture *last_bloom_blur_render_target = {};

        // blur bloom
        {
            Render_Pass_Desc bloom_pass = {};
            bloom_pass.camera_orientation = quaternion_identity();
            bloom_pass.projection_matrix = construct_orthographic_matrix(0, bloom_ping_pong_color_buffers[1].description.width, 0, bloom_ping_pong_color_buffers[1].description.height, -1, 1);
            begin_render_pass(&bloom_pass);

            set_render_targets(&bloom_ping_pong_color_buffers[1], 1, &bloom_ping_pong_depth_buffer);
            clear_bound_render_targets(v4(0, 0, 0, 1));
            bind_shaders(vertex_shader, simple_pixel_textured_shader);
            draw_texture(
                bloom_color_buffer,
                v3(0, 0, 0),
                v3(bloom_ping_pong_color_buffers[1].description.width, bloom_ping_pong_color_buffers[1].description.height, 0));

            bind_shaders(vertex_shader, blur_pixel_shader);
            for (int i = 0; i < 4; i++) {
                last_bloom_blur_render_target = &bloom_ping_pong_color_buffers[i % 2];

                Texture source_texture = bloom_ping_pong_color_buffers[(i+1) % 2];

                Blur_CBuffer blur_cbuffer = {};
                blur_cbuffer.horizontal = i % 2;
                blur_cbuffer.buffer_dimensions = v2(source_texture.description.width, source_texture.description.height);
                update_buffer(blur_cbuffer_handle, &blur_cbuffer, sizeof(Blur_CBuffer));
                bind_constant_buffers(&blur_cbuffer_handle, 1, CBS_BLUR);

                set_render_targets(last_bloom_blur_render_target, 1, &bloom_ping_pong_depth_buffer);
                clear_bound_render_targets(v4(0, 0, 0, 1));
                draw_texture(source_texture, v3(0, 0, 0), v3(last_bloom_blur_render_target->description.width, last_bloom_blur_render_target->description.height, 0));
                unset_render_targets();
            }

            unset_render_targets();
            end_render_pass();
        }

        set_render_targets(nullptr, 0, nullptr);
        clear_bound_render_targets(v4(0.39, 0.58, 0.93, 1.0f));

        Render_Pass_Desc screen_pass = {};
        screen_pass.camera_position = v3(0, 0, 0);
        screen_pass.camera_orientation = quaternion_identity();
        screen_pass.projection_matrix = construct_orthographic_matrix(0, main_window.width, 0, main_window.height, -1, 1);
        begin_render_pass(&screen_pass);

        bind_texture(*last_bloom_blur_render_target, TS_FINAL_BLOOM_MAP);
        bind_shaders(vertex_shader, final_pixel_shader);
        draw_texture(hdr_color_buffer,                 v3(0, 0, 0), v3(main_window.width, main_window.height, 0));
        bind_texture({}, TS_FINAL_BLOOM_MAP);

        bind_shaders(vertex_shader, simple_pixel_textured_shader);
        draw_texture(bloom_color_buffer,               v3(0, 0, 0), v3(128, 128, 0));
        draw_texture(bloom_ping_pong_color_buffers[0], v3(128, 0, 0), v3(256, 128, 0));
        draw_texture(bloom_ping_pong_color_buffers[1], v3(256, 0, 0), v3(384, 128, 0));

        // float z_3d = sin(time_since_startup) * sin(time_since_startup);
        float z_3d = 1;
        bind_shaders(vertex_shader, simple_pixel_3d_shader);
        draw_texture(test_3d_texture, v3(384, 0, 0), v3(512, 128, 0), z_3d);

        // printf("%f\n", z_3d);

        // printf("%f %f %f\n", camera_position.x, camera_position.y, camera_position.z);

        Vertex ffverts[1024];
        Fixed_Function ff = {};
        ff_begin(&ff, ffverts, ARRAYSIZE(ffverts));
        bind_texture(roboto_mono.texture, 0);
        bind_shaders(vertex_shader, text_pixel_shader);

        Vector3 text_pos = v3(10, main_window.height-roboto_mono.pixel_height, 0);
        const float text_size = 1;
        ff_text(&ff, "1. do_albedo_map",     roboto_mono, v4(1, 1, 1, render_options.do_albedo_map     ? 1.0 : 0.2), text_pos, text_size); text_pos.y -= roboto_mono.pixel_height * text_size;
        ff_text(&ff, "2. do_normal_map",     roboto_mono, v4(1, 1, 1, render_options.do_normal_map     ? 1.0 : 0.2), text_pos, text_size); text_pos.y -= roboto_mono.pixel_height * text_size;
        ff_text(&ff, "3. do_metallic_map",   roboto_mono, v4(1, 1, 1, render_options.do_metallic_map   ? 1.0 : 0.2), text_pos, text_size); text_pos.y -= roboto_mono.pixel_height * text_size;
        ff_text(&ff, "4. do_roughness_map",  roboto_mono, v4(1, 1, 1, render_options.do_roughness_map  ? 1.0 : 0.2), text_pos, text_size); text_pos.y -= roboto_mono.pixel_height * text_size;
        ff_text(&ff, "5. do_emission_map",   roboto_mono, v4(1, 1, 1, render_options.do_emission_map   ? 1.0 : 0.2), text_pos, text_size); text_pos.y -= roboto_mono.pixel_height * text_size;
        ff_text(&ff, "6. do_ao_map",         roboto_mono, v4(1, 1, 1, render_options.do_ao_map         ? 1.0 : 0.2), text_pos, text_size); text_pos.y -= roboto_mono.pixel_height * text_size;
        ff_text(&ff, "7. visualize_normals", roboto_mono, v4(1, 1, 1, render_options.visualize_normals ? 1.0 : 0.2), text_pos, text_size); text_pos.y -= roboto_mono.pixel_height * text_size;
        ff_end(&ff);
        end_render_pass();

        unset_render_targets();


        present(true);
    }
}
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define DEVELOPER

#include "window.h"
#include "basic.h"
#include "math.h"
#include "render_backend.h"
#include "renderer.h"
#include "game.h"

#ifdef DEVELOPER
#include "assimp_loader.cpp"
#endif

/*
TODO:
-draw commands
-particle systems?
-dynamic exposure
-instancing (do we support this already?)
-depth sorting to reduce overdraw
-transparency sorting to alpha blend properly
-dear imgui?
-point light shadows
-fix alpha blending without ruining bloom
-spot lights
-proper IBL
-cascaded shadow maps
-skeletal animation?
-AO?
*/

struct Draw_Command {
    Array<Loaded_Mesh> meshes;
    Vector3 position;
    Vector3 scale;
    Quaternion orientation;
    Vector4 color;
};

void draw_scene(Render_Options render_options, float time_since_startup, Array<Loaded_Mesh> sponza_meshes, Array<Loaded_Mesh> helmet_meshes) {
    // Quaternion helmet_orientation = axis_angle(v3(0, 1, 0), time_since_startup * 0.5);
    // draw_meshes(sponza_meshes, v3(0, 0, 0), v3(1, 1, 1), quaternion_identity(), v4(1, 1, 1, 1), render_options, false);
    draw_meshes(helmet_meshes, v3(0, 4, 0), v3(0.01, 0.01, 0.01), quaternion_identity(),    v4(1, 1, 1, 1), render_options, false);
    // draw_meshes(sponza_meshes, v3(0, 0, 0), v3(1, 1, 1), quaternion_identity(), v4(1, 1, 1, 1), render_options, true);
    // draw_meshes(helmet_meshes, v3(0, 4, 0), v3(1, 1, 1), helmet_orientation,    v4(1, 1, 1, 1), render_options, true);
}

struct Blur_CBuffer {
    int horizontal;
    Vector2 buffer_dimensions;
    float _pad;
};

#define BLOOM_BUFFER_DOWNSCALE 8.0

Texture *do_blur(Texture thing_to_blur, Texture ping_pong_buffers[2], Texture ping_pong_depth_buffer, Vertex_Shader vertex_shader, Pixel_Shader blur_pixel_shader, Pixel_Shader simple_pixel_textured_shader, Buffer blur_cbuffer_handle) {
    Render_Pass_Desc blur_pass = {};
    blur_pass.camera_orientation = quaternion_identity();
    blur_pass.projection_matrix = construct_orthographic_matrix(0, ping_pong_buffers[1].description.width, 0, ping_pong_buffers[1].description.height, -1, 1);
    begin_render_pass(&blur_pass);

    set_render_targets(&ping_pong_buffers[1], 1, &ping_pong_depth_buffer);
    clear_bound_render_targets(v4(0, 0, 0, 1));
    bind_shaders(vertex_shader, simple_pixel_textured_shader);
    draw_texture(
        thing_to_blur,
        v3(0, 0, 0),
        v3(ping_pong_buffers[1].description.width, ping_pong_buffers[1].description.height, 0));

    Texture *last_render_target = {};
    bind_shaders(vertex_shader, blur_pixel_shader);
    for (int i = 0; i < 4; i++) {
        last_render_target = &ping_pong_buffers[i % 2];

        Texture source_texture = ping_pong_buffers[(i+1) % 2];

        Blur_CBuffer blur_cbuffer = {};
        blur_cbuffer.horizontal = i % 2;
        blur_cbuffer.buffer_dimensions = v2(source_texture.description.width, source_texture.description.height);
        update_buffer(blur_cbuffer_handle, &blur_cbuffer, sizeof(Blur_CBuffer));
        bind_constant_buffers(&blur_cbuffer_handle, 1, CBS_BLUR);

        set_render_targets(last_render_target, 1, &ping_pong_depth_buffer);
        clear_bound_render_targets(v4(0, 0, 0, 1));
        draw_texture(source_texture, v3(0, 0, 0), v3(last_render_target->description.width, last_render_target->description.height, 0));
        unset_render_targets();
    }

    unset_render_targets();
    end_render_pass();
    return last_render_target;
}



void main() {
    init_platform();

    Window main_window = create_window(1920, 1080);

    init_render_backend(&main_window);
    init_renderer(&main_window);

    Texture_Description test_3d_texture_description = {};
    test_3d_texture_description.width  = 128;
    test_3d_texture_description.height = 128;
    test_3d_texture_description.depth  = 128;
    test_3d_texture_description.uav = true;
    test_3d_texture_description.type = TT_3D;
    test_3d_texture_description.format = TF_R32G32B32A32_FLOAT;
    Texture test_3d_texture = create_texture(test_3d_texture_description);

    Vertex_Shader vertex_shader                = compile_vertex_shader_from_file(L"vertex.hlsl");
    Vertex_Shader skybox_vertex_shader         = compile_vertex_shader_from_file(L"skybox_vertex.hlsl");
    Pixel_Shader  pixel_shader                 = compile_pixel_shader_from_file(L"pixel.hlsl");
    Pixel_Shader  simple_pixel_shader          = compile_pixel_shader_from_file(L"simple_pixel.hlsl");
    Pixel_Shader  simple_pixel_textured_shader = compile_pixel_shader_from_file(L"simple_pixel_textured.hlsl");
    Pixel_Shader  simple_pixel_3d_shader       = compile_pixel_shader_from_file(L"simple_pixel_3d.hlsl");
    Pixel_Shader  text_pixel_shader            = compile_pixel_shader_from_file(L"text_pixel.hlsl");
    Pixel_Shader  shadow_pixel_shader          = compile_pixel_shader_from_file(L"shadow_pixel.hlsl");
    Pixel_Shader  blur_pixel_shader            = compile_pixel_shader_from_file(L"blur_pixel.hlsl");
    Pixel_Shader  final_pixel_shader           = compile_pixel_shader_from_file(L"final_pixel.hlsl");
    Pixel_Shader  skybox_pixel_shader          = compile_pixel_shader_from_file(L"skybox_pixel.hlsl");

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

    u32 cube_indices[36] = {
         0,  2,  1,  0,  3,  2,
         4,  5,  6,  4,  6,  7,
         8, 10,  9,  8, 11, 10,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 22, 21, 20, 23, 22,
    };

    Vertex cube_vertices[24] = {
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

    Buffer cube_vertex_buffer = create_buffer(BT_VERTEX, cube_vertices, sizeof(cube_vertices[0]) * 24);
    Buffer cube_index_buffer  = create_buffer(BT_INDEX,  cube_indices,  sizeof(cube_indices[0])  * 36);

    int skybox_width;
    int skybox_height;
    byte *skybox_faces[6] = {
        load_texture_data_from_file("skybox_clouds_+x.png", &skybox_width, &skybox_height),
        load_texture_data_from_file("skybox_clouds_-x.png", &skybox_width, &skybox_height),
        load_texture_data_from_file("skybox_clouds_+y.png", &skybox_width, &skybox_height),
        load_texture_data_from_file("skybox_clouds_-y.png", &skybox_width, &skybox_height),
        load_texture_data_from_file("skybox_clouds_+z.png", &skybox_width, &skybox_height),
        load_texture_data_from_file("skybox_clouds_-z.png", &skybox_width, &skybox_height),
    };

    Texture_Description skybox_desc = {};
    skybox_desc.width  = skybox_width;
    skybox_desc.height = skybox_height;
    skybox_desc.uav = true;
    skybox_desc.type = TT_CUBEMAP;
    skybox_desc.format = TF_R8G8B8A8_UINT;
    Texture skybox_texture = create_texture(skybox_desc);
    set_cubemap_textures(skybox_texture, skybox_faces);

    for (int idx = 0; idx < ARRAYSIZE(skybox_faces); idx++) {
        delete_texture_data(skybox_faces[idx]);
    }

    Font roboto_mono = load_font_from_file("fonts/roboto_mono.ttf", 32);
    Font roboto      = load_font_from_file("fonts/roboto.ttf", 32);

    byte white_texture_data[16] = {
        255, 255, 255, 255,
        255, 255, 255, 255,
        255, 255, 255, 255,
        255, 255, 255, 255,
    };
    Texture_Description white_texture_description = {};
    white_texture_description.width = 2;
    white_texture_description.height = 2;
    white_texture_description.color_data = white_texture_data;
    Texture white_texture = create_texture(white_texture_description);

    byte black_texture_data[16] = {
        0, 0, 0, 255,
        0, 0, 0, 255,
        0, 0, 0, 255,
        0, 0, 0, 255,
    };
    Texture_Description black_texture_description = {};
    black_texture_description.width = 2;
    black_texture_description.height = 2;
    black_texture_description.color_data = black_texture_data;
    Texture black_texture = create_texture(black_texture_description);

    Render_Options render_options = {};
    render_options.do_albedo_map    = true;
    render_options.do_normal_map    = true;
    render_options.do_metallic_map  = true;
    render_options.do_roughness_map = true;
    render_options.do_emission_map  = true;
    render_options.do_ao_map        = true;

    Array<Loaded_Mesh> helmet_meshes = {};
    helmet_meshes.allocator = default_allocator();
    // load_mesh_from_file("sponza/DamagedHelmet.gltf", &helmet_meshes);

    Array<Loaded_Mesh> sponza_meshes = {};
    sponza_meshes.allocator = default_allocator();
    // load_mesh_from_file("sponza/sponza.glb", &sponza_meshes);

    Array<Loaded_Mesh> ship_meshes = {};
    ship_meshes.allocator = default_allocator();
    load_mesh_from_file("SM_Ship_Stealth_02.fbx", &ship_meshes);
    ship_meshes[0].material.albedo_map = create_texture_from_file("PolygonSciFiSpace_Texture_01_A.png", TF_R8G8B8A8_UINT_SRGB, TWM_LINEAR_WRAP);
    ship_meshes[0].material.metallic  = 1.0;
    ship_meshes[0].material.roughness = 0.35;

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


    bool freecam = false;

    Loaded_Mesh cube_loaded_mesh = {};
    cube_loaded_mesh.vertex_buffer = cube_vertex_buffer;
    cube_loaded_mesh.num_vertices = ARRAYSIZE(cube_vertices);
    cube_loaded_mesh.index_buffer = cube_index_buffer;
    cube_loaded_mesh.num_indices = ARRAYSIZE(cube_indices);
    cube_loaded_mesh.has_material = true;
    cube_loaded_mesh.material.cbuffer_handle = create_pbr_material_cbuffer();
    cube_loaded_mesh.material.ambient = 0.5;
    cube_loaded_mesh.material.roughness = 0.3;
    cube_loaded_mesh.material.metallic = 1;
    Array<Loaded_Mesh> cube_model = make_array<Loaded_Mesh>(default_allocator());
    cube_model.append(cube_loaded_mesh);



    Game_State game_state = {};
    init_game_state(&game_state);



    Array<Draw_Command> render_queue = make_array<Draw_Command>(default_allocator(), 16);

    const float FIXED_DT = 1.0f / 120;

    float time_since_startup = 0;
    const double time_at_startup = time_now();
    double last_frame_start_time = time_now();
    while (!main_window.should_close) {
        double this_frame_start_time = time_now();
        time_since_startup = (float)(this_frame_start_time - time_at_startup);
        defer(last_frame_start_time = this_frame_start_time);
        // todo(josh): proper fixed dt
        float dt = (float)(this_frame_start_time - last_frame_start_time);

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

        update_game(&game_state, dt, &main_window);



        ensure_swap_chain_size(main_window.width, main_window.height);

        ensure_texture_size(&hdr_color_buffer, main_window.width, main_window.height);
        ensure_texture_size(&hdr_depth_buffer, main_window.width, main_window.height);
        ensure_texture_size(&bloom_color_buffer, main_window.width, main_window.height);
        for (int i = 0; i < ARRAYSIZE(bloom_ping_pong_color_buffers); i++) {
            ensure_texture_size(&bloom_ping_pong_color_buffers[i], main_window.width / BLOOM_BUFFER_DOWNSCALE, main_window.height / BLOOM_BUFFER_DOWNSCALE);
        }
        ensure_texture_size(&bloom_ping_pong_depth_buffer, main_window.width / BLOOM_BUFFER_DOWNSCALE, main_window.height / BLOOM_BUFFER_DOWNSCALE);



        render_queue.clear();

        For (idx, game_state.active_entities) {
            Entity *entity = game_state.active_entities[idx];
            switch (entity->kind) {
                case ENTITY_SHIP: {
                    Draw_Command command = {};
                    command.meshes = ship_meshes;
                    command.position = entity->position;
                    command.scale = v3(0.0025, 0.0025, 0.0025);
                    command.orientation = entity->orientation;
                    if (entity->ship.player_controlled) {
                        command.color = v4(1, 1, 1, 1);
                    }
                    else {
                        command.color = v4(1, 0.25, 0.25, 1);
                    }
                    render_queue.append(command);
                    break;
                }
                case ENTITY_PROJECTILE: {
                    Draw_Command command = {};
                    command.meshes = cube_model;
                    command.position = entity->position;
                    command.scale = v3(1, 1, 1);
                    command.orientation = quaternion_identity();
                    command.color = entity->projectile.color;
                    render_queue.append(command);
                    break;
                }
            }
        }



        bind_vertex_format(default_vertex_format);
        set_backface_cull(true);
        set_depth_test(true);
        set_primitive_topology(PT_TRIANGLE_LIST);
        set_alpha_blend(true);
        Vertex ffverts[1024];

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

            Foreach (command, render_queue) {
                draw_meshes(command->meshes, command->position, command->scale, command->orientation, command->color, render_options, false);
                draw_meshes(command->meshes, command->position, command->scale, command->orientation, command->color, render_options, true);
            }

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
        lighting.has_skybox_map = 1;
        lighting.skybox_color   = v4(10, 10, 10, 1);
        bind_texture(skybox_texture, TS_PBR_SKYBOX);
        update_buffer(lighting_cbuffer_handle, &lighting, sizeof(Lighting_CBuffer));
        bind_constant_buffers(&lighting_cbuffer_handle, 1, CBS_LIGHTING);

        // draw scene to hdr buffer
        {
            Texture color_buffers[2] = {
                hdr_color_buffer,
                bloom_color_buffer,
            };
            set_render_targets(color_buffers, ARRAYSIZE(color_buffers), &hdr_depth_buffer);
            clear_bound_render_targets(v4(0, 0, 0, 0));

            bind_texture(shadow_map_color_buffer, TS_PBR_SHADOW_MAP);

            Render_Pass_Desc scene_pass = {};
            scene_pass.camera_position = game_state.camera.position;
            scene_pass.camera_orientation = game_state.camera.orientation;
            scene_pass.projection_matrix = camera_projection_matrix(game_state.camera, &main_window);

            begin_render_pass(&scene_pass);

            Fixed_Function ff = {};
            bind_shaders(vertex_shader, simple_pixel_shader);

            Array<Vertex> ship_lines_vertices = make_array<Vertex>(default_allocator());
            defer(ship_lines_vertices.destroy());
            ff_begin(&ff, &ship_lines_vertices);
            For (idx, game_state.active_entities) {
                Entity *entity = game_state.active_entities[idx];
                if (entity->kind == ENTITY_SHIP) {
                    for (int j = 0; j < entity->ship.definition.num_weapons; j++) {
                        Weapon *weapon = &entity->ship.definition.weapons[j];
                        Entity *target = get_entity(&game_state, weapon->current_target_id);

                        Vector3 weapon_position = get_weapon_position(weapon, entity);
                        Vector3 weapon_direction = get_weapon_direction(weapon, entity);

                        Vector3 min_direction = {};
                        min_direction.x = weapon_direction.x * cos(to_radians(weapon->effective_angle)) - weapon_direction.z * sin(to_radians(weapon->effective_angle));
                        min_direction.z = weapon_direction.x * sin(to_radians(weapon->effective_angle)) + weapon_direction.z * cos(to_radians(weapon->effective_angle));
                        Vector3 max_direction = {};
                        max_direction.x = weapon_direction.x * cos(to_radians(-weapon->effective_angle)) - weapon_direction.z * sin(to_radians(-weapon->effective_angle));
                        max_direction.z = weapon_direction.x * sin(to_radians(-weapon->effective_angle)) + weapon_direction.z * cos(to_radians(-weapon->effective_angle));
                        ff_line(&ff, weapon_position, weapon_position + min_direction * weapon->range, v4(0, 1, 0, 1));
                        ff_line(&ff, weapon_position, weapon_position + max_direction * weapon->range, v4(0, 1, 0, 1));

                        int last_theta = -weapon->effective_angle;
                        for (int theta = last_theta + 1; theta <= weapon->effective_angle; theta += 1) {
                            Vector3 last_direction = {};
                            last_direction.x = weapon_direction.x * cos(to_radians((float)last_theta)) - weapon_direction.z * sin(to_radians((float)last_theta));
                            last_direction.z = weapon_direction.x * sin(to_radians((float)last_theta)) + weapon_direction.z * cos(to_radians((float)last_theta));
                            Vector3 direction = {};
                            direction.x = weapon_direction.x * cos(to_radians((float)theta)) - weapon_direction.z * sin(to_radians((float)theta));
                            direction.z = weapon_direction.x * sin(to_radians((float)theta)) + weapon_direction.z * cos(to_radians((float)theta));
                            ff_line(&ff, weapon_position + last_direction * weapon->range, weapon_position + direction * weapon->range, v4(0, 1, 0, 1));
                            last_theta = theta;
                        }

                        if (target) {
                            ff_line(&ff, weapon_position, target->position, v4(0, 1, 0, 1));
                        }
                    }

                    Vector3 ship_position = entity->position;
                    for (int i = 0; i < entity->ship.num_commands; i++) {
                        int command_idx = (entity->ship.command_cursor + i) % ARRAYSIZE(entity->ship.commands);
                        Unit_Command command = entity->ship.commands[command_idx];
                        switch (command.kind) {
                            case UNIT_MOVE_COMMAND: {
                                ff_line(&ff, ship_position, command.move.to, v4(0, 1, 0, 1));
                                ship_position = command.move.to;
                                break;
                            }
                            case UNIT_ROTATE_COMMAND: {
                                Vector3 dir_to_point = normalize(command.rotate.position_to_rotate_towards - ship_position);
                                Vector3 left = cross(dir_to_point, v3(0, 1, 0));
                                left *= 0.5; // make it a little smaller
                                Vector3 arrow_start = ship_position + dir_to_point;
                                ff_line(&ff, ship_position, arrow_start, v4(0, 1, 0, 1));
                                ff_line(&ff, arrow_start, arrow_start + left, v4(0, 1, 0, 1));
                                ff_line(&ff, arrow_start, arrow_start - left, v4(0, 1, 0, 1));
                                ff_line(&ff, arrow_start + left, arrow_start + dir_to_point, v4(0, 1, 0, 1));
                                ff_line(&ff, arrow_start - left, arrow_start + dir_to_point, v4(0, 1, 0, 1));
                                break;
                            }
                        }
                    }
                }
            }
            set_primitive_topology(PT_LINE_LIST);
            ff_end(&ff);
            set_primitive_topology(PT_TRIANGLE_LIST);

            bind_shaders(vertex_shader, pixel_shader);
            Foreach (command, render_queue) {
                draw_meshes(command->meshes, command->position, command->scale, command->orientation, command->color, render_options, false);
                draw_meshes(command->meshes, command->position, command->scale, command->orientation, command->color, render_options, true);
            }


            // skybox
            bind_shaders(skybox_vertex_shader, skybox_pixel_shader);
            bind_texture(skybox_texture, TS_PBR_ALBEDO);

            set_backface_cull(false);
            draw_mesh(cube_vertex_buffer, cube_index_buffer, 24, 36, game_state.camera.position, v3(1, 1, 1), quaternion_identity(), v4(3, 3, 3, 1));
            set_backface_cull(true);

            end_render_pass();
            unset_render_targets();
            bind_texture({}, TS_PBR_SHADOW_MAP);
        }

        Texture *last_bloom_blur_render_target = do_blur(bloom_color_buffer, bloom_ping_pong_color_buffers, bloom_ping_pong_depth_buffer, vertex_shader, blur_pixel_shader, simple_pixel_textured_shader, blur_cbuffer_handle);

        set_render_targets(nullptr, 0, nullptr);
        clear_bound_render_targets(v4(0.39, 0.58, 0.93, 1.0f));

        Render_Pass_Desc screen_pass = {};
        screen_pass.camera_position = v3(0, 0, 0);
        screen_pass.camera_orientation = quaternion_identity();
        screen_pass.projection_matrix = construct_orthographic_matrix(0, main_window.width, 0, main_window.height, -1, 1);
        begin_render_pass(&screen_pass);

        bind_texture(*last_bloom_blur_render_target, TS_FINAL_BLOOM_MAP);
        bind_shaders(vertex_shader, final_pixel_shader);
        draw_texture(hdr_color_buffer, v3(0, 0, 0), v3(main_window.width, main_window.height, 0));
        bind_texture({}, TS_FINAL_BLOOM_MAP);

        bind_shaders(vertex_shader, simple_pixel_textured_shader);
        draw_texture(bloom_color_buffer,               v3(0, 0, 0), v3(128, 128, 0));
        draw_texture(bloom_ping_pong_color_buffers[0], v3(128, 0, 0), v3(256, 128, 0));
        draw_texture(bloom_ping_pong_color_buffers[1], v3(256, 0, 0), v3(384, 128, 0));

        // float z_3d = sin(time_since_startup) * sin(time_since_startup);
        float z_3d = 1;
        bind_shaders(vertex_shader, simple_pixel_3d_shader);
        draw_texture(test_3d_texture, v3(384, 0, 0), v3(512, 128, 0), z_3d);

        Fixed_Function ff = {};
        Array<Vertex> text_vertices = make_array<Vertex>(default_allocator());
        defer(text_vertices.destroy());
        ff_begin(&ff, &text_vertices);
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
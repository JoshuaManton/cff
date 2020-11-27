#include <stdio.h>
#include <stdlib.h>

#define DEVELOPER

#define CFF_APPLICATION_IMPLEMENTATION
#include "application.h"
#include "basic.h"
#include "math.h"
#include "renderer.h"

#include "half.cpp"

#ifdef DEVELOPER
#include "assimp_loader.cpp"
#endif

/*
TODO:
-particle systems
-SSAO
-fix alpha blending without ruining bloom
-suggestion from martijn: color the fog based on the 1x1 downsample from last frame
-transparency sorting to alpha blend properly
-skeletal animation
-instancing (do we support this already?)
-assimp model scale
-bounding boxes/spheres for meshes/models
-depth sorting to reduce overdraw
-point light shadows?
-spot lights
-proper IBL
-cascaded shadow maps
*/

struct Draw_Command {
    Model model;
    Vector3 position;
    Vector3 scale;
    Quaternion orientation;
    Vector4 color;
};

struct Final_CBuffer {
    float exposure;
    float pad[3];
};

#define BLOOM_BUFFER_DOWNSCALE 8.0

void draw_render_options_window(Render_Options *render_options);

void main() {
    init_platform();
    Window main_window = create_window(1920, 1080);
    init_render_backend(&main_window);
    init_renderer(&main_window);



    Vertex_Shader vertex_shader                = compile_vertex_shader_from_file(L"vertex.hlsl");
    Vertex_Shader skybox_vertex_shader         = compile_vertex_shader_from_file(L"skybox_vertex.hlsl");
    Pixel_Shader  pixel_shader                 = compile_pixel_shader_from_file(L"pixel.hlsl");
    Pixel_Shader  simple_pixel_shader          = compile_pixel_shader_from_file(L"simple_pixel.hlsl");
    Pixel_Shader  simple_textured_pixel_shader = compile_pixel_shader_from_file(L"simple_pixel_textured.hlsl");
    Pixel_Shader  simple_pixel_3d_shader       = compile_pixel_shader_from_file(L"simple_pixel_3d.hlsl");
    Pixel_Shader  text_pixel_shader            = compile_pixel_shader_from_file(L"text_pixel.hlsl");
    Pixel_Shader  depth_pixel_shader           = compile_pixel_shader_from_file(L"depth_pixel.hlsl");
    Pixel_Shader  blur_pixel_shader            = compile_pixel_shader_from_file(L"blur_pixel.hlsl");
    Pixel_Shader  final_pixel_shader           = compile_pixel_shader_from_file(L"final_pixel.hlsl");
    Pixel_Shader  skybox_pixel_shader          = compile_pixel_shader_from_file(L"skybox_pixel.hlsl");
    Pixel_Shader  ssr_pixel_shader             = compile_pixel_shader_from_file(L"ssr_pixel.hlsl");

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
    Model cube_model = create_model(default_allocator());
    cube_model.meshes.append(cube_loaded_mesh);

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



    Texture shadow_map_color_buffer = {};
    Texture shadow_map_depth_buffer = {};
    Texture_Description shadow_map_description = {};
    shadow_map_description.width = 2048;
    shadow_map_description.height = 2048;
    // todo(josh): this should just be one component per pixel
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
    create_color_and_depth_buffers(hdr_description, &hdr_color_buffer, &hdr_depth_buffer);

    Texture_Description final_composite_desc = {};
    final_composite_desc.width = main_window.width;
    final_composite_desc.height = main_window.height;
    final_composite_desc.format = TF_R16G16B16A16_FLOAT;
    final_composite_desc.wrap_mode = TWM_LINEAR_CLAMP;
    Texture final_composite_color_buffer = {};
    Texture final_composite_depth_buffer = {};
    create_color_and_depth_buffers(final_composite_desc, &final_composite_color_buffer, &final_composite_depth_buffer);

    Texture_Description gbuffer_normals_desc = {};
    gbuffer_normals_desc.width = main_window.width;
    gbuffer_normals_desc.height = main_window.height;
    gbuffer_normals_desc.format = TF_R16G16B16A16_FLOAT; // todo(josh): do we really need this format? would r8g8b8a8_uint suffice?
    gbuffer_normals_desc.render_target = true;
    Texture gbuffer_normals = create_texture(gbuffer_normals_desc);

    Texture_Description gbuffer_positions_desc = {};
    gbuffer_positions_desc.width = main_window.width;
    gbuffer_positions_desc.height = main_window.height;
    gbuffer_positions_desc.format = TF_R16G16B16A16_FLOAT;
    gbuffer_positions_desc.render_target = true;
    Texture gbuffer_positions = create_texture(gbuffer_positions_desc);

    Texture_Description gbuffer_metal_roughness_desc = {};
    gbuffer_metal_roughness_desc.width = main_window.width;
    gbuffer_metal_roughness_desc.height = main_window.height;
    gbuffer_metal_roughness_desc.format = TF_R8G8B8A8_UINT;
    gbuffer_metal_roughness_desc.render_target = true;
    Texture gbuffer_metal_roughness = create_texture(gbuffer_metal_roughness_desc);

    Texture_Description bloom_desc = {};
    bloom_desc.width  = main_window.width;
    bloom_desc.height = main_window.height;
    bloom_desc.type = TT_2D;
    bloom_desc.format = TF_R16G16B16A16_FLOAT;
    bloom_desc.wrap_mode = TWM_LINEAR_CLAMP;
    bloom_desc.render_target = true;
    Texture bloom_color_buffer = create_texture(bloom_desc);

    Texture_Description ssr_color_buffer_desc = {};
    ssr_color_buffer_desc.width  = main_window.width;
    ssr_color_buffer_desc.height = main_window.height;
    ssr_color_buffer_desc.type = TT_2D;
    ssr_color_buffer_desc.format = TF_R16G16B16A16_FLOAT;
    ssr_color_buffer_desc.wrap_mode = TWM_LINEAR_CLAMP;
    ssr_color_buffer_desc.render_target = true;
    Texture ssr_color_buffer = create_texture(ssr_color_buffer_desc);

    Blurrer blurrer = make_blurrer(main_window.width / BLOOM_BUFFER_DOWNSCALE, main_window.height / BLOOM_BUFFER_DOWNSCALE, vertex_shader, blur_pixel_shader, simple_textured_pixel_shader);

    // todo(josh): do we need to do every single power of two?
    int auto_exposure_downsample_sizes[10] = {
        512, 256, 128, 64, 32, 16, 8, 4, 2, 1,
    };

    Texture_Description auto_exposure_downsample_desc = {};
    auto_exposure_downsample_desc.wrap_mode = TWM_LINEAR_CLAMP;
    auto_exposure_downsample_desc.format = TF_R16G16B16A16_FLOAT;
    auto_exposure_downsample_desc.render_target = true;
    Texture auto_exposure_downsample_buffers[ARRAYSIZE(auto_exposure_downsample_sizes)] = {};
    for (int i = 0; i < ARRAYSIZE(auto_exposure_downsample_buffers); i++) {
        Texture_Description desc = auto_exposure_downsample_desc;
        desc.width  = auto_exposure_downsample_sizes[i];
        desc.height = auto_exposure_downsample_sizes[i];
        auto_exposure_downsample_buffers[i] = create_texture(desc);
    }
    Texture_Description auto_exposure_cpu_read_buffer_desc = auto_exposure_downsample_desc;
    auto_exposure_cpu_read_buffer_desc.width  = 8;
    auto_exposure_cpu_read_buffer_desc.height = 8;
    auto_exposure_cpu_read_buffer_desc.render_target = false;
    auto_exposure_cpu_read_buffer_desc.cpu_read_target = true;
    Texture auto_exposure_cpu_read_buffer = create_texture(auto_exposure_cpu_read_buffer_desc);



    Buffer lighting_cbuffer_handle = create_buffer(BT_CONSTANT, nullptr, sizeof(Lighting_CBuffer));
    Buffer ssr_cbuffer_handle      = create_buffer(BT_CONSTANT, nullptr, sizeof(SSR_CBuffer));
    Buffer final_cbuffer_handle    = create_buffer(BT_CONSTANT, nullptr, sizeof(Final_CBuffer));



    Render_Options render_options = {};
    render_options.do_albedo_map    = true;
    render_options.do_normal_map    = true;
    render_options.do_metallic_map  = true;
    render_options.do_roughness_map = true;
    render_options.do_emission_map  = true;
    render_options.do_ao_map        = true;
    render_options.bloom_radius = 10;
    render_options.bloom_iterations = 2;
    render_options.exposure_modifier = 0.1;
    render_options.bloom_threshold = 10;
    render_options.ambient_modifier = 1;
    render_options.sun_color = v3(1, 0.7, 0.3);
    render_options.sun_intensity = 200;
    render_options.fog_color = v3(1, 0.7, 0.3);

    Model helmet_model = load_model_from_file("sponza/DamagedHelmet.gltf", default_allocator());
    Model sponza_model = load_model_from_file("sponza/sponza.glb", default_allocator());
    // sponza_model.meshes[8].material.roughness = 0.2;


    Vector3 camera_position = {};
    Quaternion camera_orientation = quaternion_identity();


    Array<Draw_Command> render_queue = make_array<Draw_Command>(default_allocator(), 16);

    float current_exposure = 0.25;

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

        dear_imgui_new_frame(&main_window, dt);

        draw_render_options_window(&render_options);

        const float CAMERA_SPEED_BASE = 3;
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



        ensure_swap_chain_size(main_window.width, main_window.height);

        ensure_texture_size(&hdr_color_buffer, main_window.width, main_window.height);
        ensure_texture_size(&hdr_depth_buffer, main_window.width, main_window.height);
        ensure_texture_size(&final_composite_color_buffer, main_window.width, main_window.height);
        ensure_texture_size(&final_composite_depth_buffer, main_window.width, main_window.height);
        // ensure_texture_size(&ssao_depth_color_buffer, main_window.width, main_window.height);
        ensure_texture_size(&gbuffer_positions, main_window.width, main_window.height);
        ensure_texture_size(&gbuffer_metal_roughness, main_window.width, main_window.height);
        ensure_texture_size(&gbuffer_normals, main_window.width, main_window.height);
        ensure_texture_size(&bloom_color_buffer, main_window.width, main_window.height);
        ensure_texture_size(&ssr_color_buffer, main_window.width, main_window.height);
        ensure_blurrer_texture_sizes(&blurrer, main_window.width / BLOOM_BUFFER_DOWNSCALE, main_window.height / BLOOM_BUFFER_DOWNSCALE);



        render_queue.clear();

        Draw_Command helmet_draw_command = {};
        helmet_draw_command.model = helmet_model;
        helmet_draw_command.position = v3(0, 4, 0);
        helmet_draw_command.orientation = axis_angle(v3(0, 1, 0), time_since_startup * 0.5);
        helmet_draw_command.scale = v3(1, 1, 1);
        helmet_draw_command.color = v4(1, 1, 1, 1);
        render_queue.append(helmet_draw_command);

        Draw_Command sponza_draw_command = {};
        sponza_draw_command.model = sponza_model;
        sponza_draw_command.position = v3(0, 0, 0);
        sponza_draw_command.orientation = quaternion_identity();
        sponza_draw_command.scale = v3(1, 1, 1);
        sponza_draw_command.color = v4(1, 1, 1, 1);
        render_queue.append(sponza_draw_command);

        Fixed_Function ff = {};
        Array<Vertex> ff_vertices = make_array<Vertex>(default_allocator()); // @Alloc
        defer(ff_vertices.destroy());
        ff_begin(&ff, &ff_vertices);



        bind_vertex_format(default_vertex_format);
        set_backface_cull(true);
        set_depth_test(true);
        set_primitive_topology(PT_TRIANGLE_LIST);
        set_alpha_blend(true);

        Matrix4 sun_transform = {};

        Quaternion sun_orientation = axis_angle(v3(0, 1, 0), to_radians(90 + sin(time_since_startup * 0.04) * 30)) * axis_angle(v3(1, 0, 0), to_radians(90 + sin(time_since_startup * 0.043) * 30));
        // Quaternion sun_orientation = axis_angle(v3(0, 1, 0), to_radians(90)) * axis_angle(v3(1, 0, 0), to_radians(90));

        // draw scene to shadow map
        {
            Render_Pass_Desc shadow_pass = {};
            shadow_pass.render_target_bindings.color_bindings[0] = {shadow_map_color_buffer, true, v4(0, 0, 0, 0)};
            shadow_pass.render_target_bindings.depth_binding     = {shadow_map_depth_buffer, true, 1};
            shadow_pass.camera_position = v3(0, 20, 0);
            shadow_pass.camera_orientation = sun_orientation;
            shadow_pass.projection_matrix = construct_orthographic_matrix(-20, 20, -20, 20, -100, 100);
            sun_transform = shadow_pass.projection_matrix * construct_view_matrix(shadow_pass.camera_position, shadow_pass.camera_orientation);
            begin_render_pass(&shadow_pass);
            defer(end_render_pass());
            bind_shaders(vertex_shader, depth_pixel_shader);

            Foreach (command, render_queue) {
                draw_model(command->model, command->position, command->scale, command->orientation, command->color, render_options, false);
                draw_model(command->model, command->position, command->scale, command->orientation, command->color, render_options, true);
            }
        }

        Vector4 skybox_color = v4(10, 10, 10, 1);

        Lighting_CBuffer lighting = {};
        // lighting.point_light_positions[lighting.num_point_lights] = v4(sin(time_since_startup) * 3, 6, 0, 1);
        // lighting.point_light_colors[lighting.num_point_lights++]  = v4(1, 0, 0, 1) * 500;
        // lighting.point_light_positions[lighting.num_point_lights] = v4(sin(time_since_startup * 0.6) * 3, 6, 0, 1);
        // lighting.point_light_colors[lighting.num_point_lights++]  = v4(0, 1, 0, 1) * 500;
        // lighting.point_light_positions[lighting.num_point_lights] = v4(sin(time_since_startup * 0.7) * 3, 6, 0, 1);
        // lighting.point_light_colors[lighting.num_point_lights++]  = v4(0, 0, 1, 1) * 500;
        lighting.sun_direction = quaternion_forward(sun_orientation);
        lighting.sun_color = render_options.sun_color * render_options.sun_intensity;
        lighting.sun_transform = sun_transform;
        lighting.fog_base_color = render_options.fog_color;
        lighting.fog_density    = 0.05;
        lighting.fog_y_level    = -1;
        lighting.has_skybox_map = 1;
        lighting.skybox_color   = skybox_color;
        lighting.bloom_threshold = render_options.bloom_threshold;
        lighting.ambient_modifier = render_options.ambient_modifier;
        bind_texture(skybox_texture, TS_PBR_SKYBOX);
        update_buffer(lighting_cbuffer_handle, &lighting, sizeof(Lighting_CBuffer));
        bind_constant_buffers(&lighting_cbuffer_handle, 1, CBS_LIGHTING);

        // draw scene
        {
            Render_Pass_Desc scene_pass_desc = {};
            scene_pass_desc.camera_position = camera_position;
            scene_pass_desc.camera_orientation = camera_orientation;
            scene_pass_desc.projection_matrix = construct_perspective_matrix(to_radians(60), (float)main_window.width / (float)main_window.height, 0.01, 1000);

            // draw scene to hdr buffer
            {
                Render_Pass_Desc scene_pass = scene_pass_desc;
                scene_pass.render_target_bindings.color_bindings[0] = {hdr_color_buffer,        true};
                scene_pass.render_target_bindings.color_bindings[1] = {bloom_color_buffer,      true};
                scene_pass.render_target_bindings.color_bindings[2] = {gbuffer_positions,       true};
                scene_pass.render_target_bindings.color_bindings[3] = {gbuffer_normals,         true};
                scene_pass.render_target_bindings.color_bindings[4] = {gbuffer_metal_roughness, true};
                scene_pass.render_target_bindings.depth_binding     = {hdr_depth_buffer,        true, 1};
                begin_render_pass(&scene_pass);
                defer(end_render_pass());
                bind_texture(shadow_map_color_buffer, TS_PBR_SHADOW_MAP);
                defer(bind_texture({}, TS_PBR_SHADOW_MAP));
                bind_shaders(vertex_shader, pixel_shader);
                Foreach (command, render_queue) {
                    draw_model(command->model, command->position, command->scale, command->orientation, command->color, render_options, false);
                    draw_model(command->model, command->position, command->scale, command->orientation, command->color, render_options, true);
                }

                // skybox
                bind_shaders(skybox_vertex_shader, skybox_pixel_shader);
                bind_texture(skybox_texture, TS_PBR_ALBEDO);

                set_backface_cull(false);
                draw_mesh(cube_vertex_buffer, cube_index_buffer, ARRAYSIZE(cube_vertices), ARRAYSIZE(cube_indices), camera_position, v3(1, 1, 1), quaternion_identity(), skybox_color);
                set_backface_cull(true);

                // ff_line_circle(&ff, v3(0, 1, 0), 1, v3(0, 1, 0), v4(5, 0, 0, 1));
                // set_primitive_topology(PT_LINE_LIST);
                // bind_shaders(vertex_shader, simple_pixel_shader);
                // ff_end(&ff);
                // set_primitive_topology(PT_TRIANGLE_LIST);
            }

            // do screen-space reflections
            {
                SSR_CBuffer ssr_cbuffer = {};
                ssr_cbuffer.scene_camera_position = scene_pass_desc.camera_position;
                ssr_cbuffer.camera_matrix = scene_pass_desc.projection_matrix * construct_view_matrix(scene_pass_desc.camera_position, scene_pass_desc.camera_orientation);
                ssr_cbuffer.inverse_camera_matrix = inverse(ssr_cbuffer.camera_matrix);
                update_buffer(ssr_cbuffer_handle, &ssr_cbuffer, sizeof(SSR_CBuffer));
                bind_constant_buffers(&ssr_cbuffer_handle, 1, CBS_SSR);

                Render_Pass_Desc ssr_pass = {};
                ssr_pass.render_target_bindings.color_bindings[0] = {ssr_color_buffer, true};
                ssr_pass.render_target_bindings.depth_binding     = {{},               true, 1}; // todo(josh): we really shouldn't be binding a depth buffer here
                ssr_pass.camera_position = v3(0, 0, 0);
                ssr_pass.camera_orientation = quaternion_identity();
                ssr_pass.projection_matrix = construct_orthographic_matrix(0, main_window.width, 0, main_window.height, -1, 1);
                begin_render_pass(&ssr_pass);
                defer(end_render_pass());

                bind_texture(gbuffer_normals, TS_SSR_NORMAL_MAP);
                defer(bind_texture({}, TS_SSR_NORMAL_MAP));
                bind_texture(gbuffer_positions, TS_SSR_POSITIONS_MAP);
                defer(bind_texture({}, TS_SSR_POSITIONS_MAP));
                bind_texture(gbuffer_metal_roughness, TS_SSR_METAL_ROUGHNESS_MAP);
                defer(bind_texture({}, TS_SSR_POSITIONS_MAP));

                bind_shaders(vertex_shader, ssr_pixel_shader);
                draw_texture(hdr_color_buffer, v3(0, 0, 0), v3(main_window.width, main_window.height, 0));
            }
        }

        Texture last_bloom_blur_render_target = do_blur(&blurrer, bloom_color_buffer, render_options.bloom_radius, render_options.bloom_iterations);
        // Texture last_ssr_blur_render_target = do_blur(&blurrer, ssr_color_buffer, 2, 1);

        {
            // todo(josh): abstract this d3d dependency
            // todo(josh): abstract this d3d dependency
            // todo(josh): abstract this d3d dependency

            D3D11_MAPPED_SUBRESOURCE texture_resource = {};
            auto result = directx.device_context->Map(*((ID3D11Resource **)&auto_exposure_cpu_read_buffer.backend.handle_2d), 0, D3D11_MAP_READ, 0, &texture_resource);
            assert(result == S_OK);

            int pixel_size = texture_format_infos[auto_exposure_cpu_read_buffer.description.format].pixel_size_in_bytes;
            assert(pixel_size == sizeof(u64));
            int pixels_length_in_bytes = auto_exposure_cpu_read_buffer.description.width * auto_exposure_cpu_read_buffer.description.height * pixel_size;
            int pixels_length_in_u64 = pixels_length_in_bytes / 8;
            u64 *pixels = (u64 *)texture_resource.pData;
            u64 middle_pixel = pixels[4 + (auto_exposure_cpu_read_buffer.description.width * 4)];
            u16 middle_pixel_r = (u16)(middle_pixel >> 0);
            u16 middle_pixel_g = (u16)(middle_pixel >> 16);
            u16 middle_pixel_b = (u16)(middle_pixel >> 32);
            u32 ru32 = half_to_float(middle_pixel_r);
            float r = *((float *)(&ru32));
            u32 gu32 = half_to_float(middle_pixel_g);
            float g = *((float *)(&gu32));
            u32 bu32 = half_to_float(middle_pixel_b);
            float b = *((float *)(&bu32));

            directx.device_context->Unmap(*((ID3D11Resource **)&auto_exposure_cpu_read_buffer.backend.handle_2d), 0);

            Vector3 color = v3(r*r, g*g, b*b);
            float brightness = dot(color, v3(0.2, 0.7, 0.1)); // todo(josh): @CorrectBrightness (0.2, 0.7, 0.1) are not exact. martijn: "if you want the exact ones, look at wikipedia at the Y component of the RGB primaries of the sRGB color space"
            float exposure_this_frame = render_options.exposure_modifier / (brightness + 1e-3);

            if (abs(current_exposure - exposure_this_frame) > 0.1) {
                const float EXPOSURE_SPEED = 0.5;
                if (current_exposure < exposure_this_frame) {
                    current_exposure += EXPOSURE_SPEED * dt;
                    if (current_exposure > exposure_this_frame) {
                        current_exposure = exposure_this_frame;
                    }
                }
                else if (current_exposure > exposure_this_frame) {
                    current_exposure -= EXPOSURE_SPEED * dt;
                    if (current_exposure < exposure_this_frame) {
                        current_exposure = exposure_this_frame;
                    }
                }
            }
            else {
                current_exposure = lerp(current_exposure, exposure_this_frame, 1 * dt);
            }
        }

        // make the final composite
        {
            Render_Pass_Desc screen_pass = {};
            screen_pass.render_target_bindings.color_bindings[0] = {final_composite_color_buffer, true, v4(0, 0, 0, 0)};
            screen_pass.render_target_bindings.depth_binding     = {final_composite_depth_buffer, true, 1};
            screen_pass.camera_position = v3(0, 0, 0);
            screen_pass.camera_orientation = quaternion_identity();
            screen_pass.projection_matrix = construct_orthographic_matrix(0, main_window.width, 0, main_window.height, -1, 1);
            begin_render_pass(&screen_pass);
            defer(end_render_pass());

            bind_texture(last_bloom_blur_render_target, TS_FINAL_BLOOM_MAP);
            defer(bind_texture({}, TS_FINAL_BLOOM_MAP));
            bind_texture(ssr_color_buffer, TS_FINAL_SSR_MAP);
            defer(bind_texture({}, TS_FINAL_SSR_MAP));

            bind_shaders(vertex_shader, final_pixel_shader);

            Final_CBuffer final_cbuffer = {};
            final_cbuffer.exposure = current_exposure;
            if (render_options.visualize_normals) {
                final_cbuffer.exposure = 1;
            }
            update_buffer(final_cbuffer_handle, &final_cbuffer, sizeof(Final_CBuffer));
            bind_constant_buffers(&final_cbuffer_handle, 1, CBS_FINAL);
            draw_texture(hdr_color_buffer, v3(0, 0, 0), v3(main_window.width, main_window.height, 0));
        }

        // calculate exposure brightness for next frame
        {
            set_alpha_blend(false);
            defer(set_alpha_blend(true));
            bind_shaders(vertex_shader, simple_textured_pixel_shader);
            Texture thing_to_draw = final_composite_color_buffer;
            for (int i = 0; i < ARRAYSIZE(auto_exposure_downsample_buffers); i++) {
                Render_Pass_Desc screen_pass = {};
                screen_pass.render_target_bindings.color_bindings[0] = {auto_exposure_downsample_buffers[i], true, v4(0, 0, 0, 0)};
                screen_pass.render_target_bindings.depth_binding     = {{}, true, 1};
                screen_pass.camera_position = v3(0, 0, 0);
                screen_pass.camera_orientation = quaternion_identity();
                screen_pass.projection_matrix = construct_orthographic_matrix(0, thing_to_draw.description.width, 0, thing_to_draw.description.height, -1, 1);
                begin_render_pass(&screen_pass);
                defer(end_render_pass());
                draw_texture(thing_to_draw, v3(0, 0, 0), v3(thing_to_draw.description.width, thing_to_draw.description.height, 0));
                thing_to_draw = auto_exposure_downsample_buffers[i];
            }
            copy_texture(auto_exposure_cpu_read_buffer, auto_exposure_downsample_buffers[6]);
        }

        // actually draw to the screen
        {
            Render_Pass_Desc screen_pass = {};
            screen_pass.render_target_bindings.color_bindings[0] = {{}, true, v4(0.39, 0.58, 0.93, 1.0f)};
            screen_pass.render_target_bindings.depth_binding     = {{}, true, 1};
            screen_pass.camera_position = v3(0, 0, 0);
            screen_pass.camera_orientation = quaternion_identity();
            screen_pass.projection_matrix = construct_orthographic_matrix(0, main_window.width, 0, main_window.height, -1, 1);
            begin_render_pass(&screen_pass);
            defer(end_render_pass());

            bind_shaders(vertex_shader, simple_textured_pixel_shader);
            draw_texture(final_composite_color_buffer,        v3(0, 0, 0),   v3(main_window.width, main_window.height, 0));

            /*
            draw_texture(bloom_color_buffer,                  v3(0, 0, 0),   v3(128, 128, 0));
            draw_texture(last_bloom_blur_render_target,       v3(128, 0, 0), v3(256, 128, 0));
            draw_texture(ssr_color_buffer,                    v3(256, 0, 0), v3(384, 128, 0));
            draw_texture(gbuffer_positions,                   v3(384, 0, 0), v3(512, 128, 0));
            draw_texture(gbuffer_normals,                     v3(512, 0, 0), v3(640, 128, 0));
            draw_texture(gbuffer_metal_roughness,             v3(640, 0, 0), v3(768, 128, 0));
            draw_texture(auto_exposure_downsample_buffers[6], v3(768, 0, 0), v3(896, 128, 0));

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
            */

            dear_imgui_render(true);
        }

        present(true);
    }
}

void draw_render_options_window(Render_Options *render_options) {
    if (ImGui::Begin("Renderer")) {
        ImGui::Checkbox("do albedo map",     &render_options->do_albedo_map);
        ImGui::Checkbox("do normal map",     &render_options->do_normal_map);
        ImGui::Checkbox("do metallic map",   &render_options->do_metallic_map);
        ImGui::Checkbox("do roughness map",  &render_options->do_roughness_map);
        ImGui::Checkbox("do emission map",   &render_options->do_emission_map);
        ImGui::Checkbox("do ao map",         &render_options->do_ao_map);
        ImGui::Checkbox("visualize normals", &render_options->visualize_normals);

        ImGui::SliderFloat("ambient modifier",   &render_options->ambient_modifier, 0, 1);

        ImGui::SliderFloat("bloom radius",      &render_options->bloom_radius, 1, 100);
        ImGui::SliderInt("bloom iterations",    &render_options->bloom_iterations, 0, 10);
        ImGui::SliderFloat("bloom threshold",   &render_options->bloom_threshold, 0, 50);

        ImGui::SliderFloat("exposure modifier", &render_options->exposure_modifier, 0, 1);

        ImGui::SliderFloat("sun color r", &render_options->sun_color.x, 0, 1);
        ImGui::SliderFloat("sun color g", &render_options->sun_color.y, 0, 1);
        ImGui::SliderFloat("sun color b", &render_options->sun_color.z, 0, 1);
        ImGui::SliderFloat("sun intensity", &render_options->sun_intensity, 0, 500);

        ImGui::SliderFloat("fog color r", &render_options->fog_color.x, 0, 1);
        ImGui::SliderFloat("fog color g", &render_options->fog_color.y, 0, 1);
        ImGui::SliderFloat("fog color b", &render_options->fog_color.z, 0, 1);

    }
    ImGui::End();
}
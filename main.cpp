#include <stdio.h>
#include <stdlib.h>

#define DEVELOPER

#define CFF_APPLICATION_IMPLEMENTATION
#include "application.h"
#include "basic.h"
#include "math.h"
#include "renderer.h"

#ifdef DEVELOPER
#include "assimp_loader.cpp"
#endif

/*
TODO:
-fully defer lighting calculations
-bounding boxes/spheres for meshes/models
-cascaded shadow maps
-particle systems
-SSAO
-fix alpha blending without ruining bloom
-transparency sorting to alpha blend properly
-skeletal animation
-instancing (do we support this already?)
-assimp model scale
-depth sorting to reduce overdraw
-proper IBL
-point light shadows?
-spot lights
*/

void main() {
    init_platform();
    Window main_window = create_window(1920, 1080);
    init_render_backend(&main_window);
    init_renderer(&main_window);

    Renderer3D renderer = {};
    create_renderer3d(&renderer, &main_window);

    Font roboto_mono = load_font_from_file("fonts/roboto_mono.ttf", 32);
    Font roboto      = load_font_from_file("fonts/roboto.ttf", 32);

    Render_Options render_options    = {};
    render_options.do_albedo_map     = true;
    render_options.do_normal_map     = true;
    render_options.do_metallic_map   = true;
    render_options.do_roughness_map  = true;
    render_options.do_emission_map   = true;
    render_options.do_ao_map         = true;
    render_options.do_bloom          = true;
    render_options.blur_function     = BF_GAUSSIAN;
    render_options.bloom_slope       = 0.5;
    render_options.bloom_radius      = 10;
    render_options.bloom_iterations  = 2;
    render_options.bloom_threshold   = 10;
    render_options.gaussian_height   = 0.03;
    render_options.exposure_modifier = 0.1;
    render_options.ambient_modifier  = 1;
    render_options.do_shadows        = true;
    render_options.sun_color         = v3(1, 0.7, 0.3);
    render_options.sun_intensity     = 200;
    render_options.do_fog            = true;
    render_options.fog_color         = v3(1, 0.7, 0.3);
    render_options.fog_density       = 0.05;

    Model helmet_model = load_model_from_file("sponza/DamagedHelmet.gltf", default_allocator());
    Model sponza_model = load_model_from_file("sponza/sponza.glb", default_allocator());
    sponza_model.meshes[8].material.roughness = 0.2;

    Vector3 camera_position = {};
    Quaternion camera_orientation = quaternion_identity();

    Array<Draw_Command> render_queue = make_array<Draw_Command>(default_allocator(), 16);

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

        draw_render_options_editor_window(&render_options);

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

        render_queue.clear();

        Draw_Command helmet_draw_command = {};
        helmet_draw_command.model = helmet_model;
        helmet_draw_command.position = v3(0, 4, 0);
        helmet_draw_command.orientation = axis_angle(v3(0, 1, 0), to_radians(90));
        // helmet_draw_command.orientation = quaternion_identity();
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

        // render_options.sun_orientation = axis_angle(v3(0, 1, 0), to_radians(90 + sin(time_since_startup * 0.04) * 30)) * axis_angle(v3(1, 0, 0), to_radians(90 + sin(time_since_startup * 0.043) * 30));
        render_options.sun_orientation = axis_angle(v3(0, 1, 0), to_radians(60)) * axis_angle(v3(1, 0, 0), to_radians(75));

        render_scene(&renderer, render_queue, camera_position, camera_orientation, render_options, &main_window, time_since_startup, dt);
        dear_imgui_render(true);
        present(true);
    }
}
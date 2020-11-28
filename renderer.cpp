#include "renderer.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "half.cpp"

#include "external/dearimgui/imgui.h"

struct Renderer_State {
    Render_Pass_Desc *current_render_pass;
    Buffer pass_cbuffer_handle;
    Buffer model_cbuffer_handle;
};

Renderer_State renderer_state;

void init_renderer(Window *window) {
    renderer_state.pass_cbuffer_handle  = create_buffer(BT_CONSTANT, nullptr, sizeof(Pass_CBuffer));
    renderer_state.model_cbuffer_handle = create_buffer(BT_CONSTANT, nullptr, sizeof(Model_CBuffer));
}



Model create_model(Allocator allocator) {
    Model model = {};
    model.meshes = make_array<Loaded_Mesh>(allocator);
    return model;
}

void destroy_model(Model model) {
    model.meshes.destroy();
}



void flush_pbr_material(Buffer buffer, PBR_Material material, Render_Options options) {
    PBR_Material_CBuffer material_cbuffer = {};
    material_cbuffer.ambient   = material.ambient;
    material_cbuffer.metallic  = material.metallic;
    material_cbuffer.roughness = material.roughness;

    if (material.albedo_map.valid && options.do_albedo_map) {
        bind_texture(material.albedo_map, TS_PBR_ALBEDO);
        material_cbuffer.has_albedo_map = 1;
    }
    if (material.normal_map.valid && options.do_normal_map) {
        bind_texture(material.normal_map, TS_PBR_NORMAL);
        material_cbuffer.has_normal_map = 1;
    }
    if (material.metallic_map.valid && options.do_metallic_map) {
        bind_texture(material.metallic_map, TS_PBR_METALLIC);
        material_cbuffer.has_metallic_map = 1;
    }
    if (material.roughness_map.valid && options.do_roughness_map) {
        bind_texture(material.roughness_map, TS_PBR_ROUGHNESS);
        material_cbuffer.has_roughness_map = 1;
    }
    if (material.emission_map.valid && options.do_emission_map) {
        bind_texture(material.emission_map, TS_PBR_EMISSION);
        material_cbuffer.has_emission_map = 1;
    }
    if (material.ao_map.valid && options.do_ao_map) {
        bind_texture(material.ao_map, TS_PBR_AO);
        material_cbuffer.has_ao_map = 1;
    }

    update_buffer(buffer, &material_cbuffer, sizeof(PBR_Material_CBuffer));
    bind_constant_buffers(&buffer, 1, CBS_MATERIAL);
}

Buffer create_pbr_material_cbuffer() {
    Buffer buffer = create_buffer(BT_CONSTANT, nullptr, sizeof(PBR_Material_CBuffer));
    return buffer;
}



Font load_font_from_file(char *filename, float size) {
    int ttf_data_len;
    unsigned char *ttf_data = (unsigned char *)read_entire_file(filename, &ttf_data_len);
    if (!ttf_data) {
        printf("load_font_from_file() couldn't find file: %s\n", filename);
        return {};
    }
    defer(free(ttf_data));

    Font font = {};
    font.pixel_height = size;
    font.dim = 256;
    byte *pixels = nullptr;
    defer(free(default_allocator(), pixels));
    int tries = 0;
    while (true) {
        if (tries >= 5) {
            assert(false && "Failed to create font after a bunch of tries.");
        }
        tries += 1;
        // todo(josh): check for max texture size for GPU?

        pixels = (byte *)alloc(default_allocator(), font.dim * font.dim);
        int ret = stbtt_BakeFontBitmap(ttf_data, 0, size, pixels, font.dim, font.dim, 0, ARRAYSIZE(font.chars), font.chars);
        if (ret < 0) {
            free(default_allocator(), pixels);
            font.dim = (int)((f32)font.dim * 1.5);
        }
        else {
            break;
        }
    }

    assert(pixels != nullptr);

    Texture_Description desc = {};
    desc.type = TT_2D;
    desc.width = font.dim;
    desc.height = font.dim;
    desc.format = TF_R8_UINT;
    desc.wrap_mode = TWM_LINEAR_CLAMP;
    desc.color_data = pixels;
    font.texture = create_texture(desc);
    return font;
}

void destroy_font(Font font) {
    destroy_texture(font.texture);
}



void begin_render_pass(Render_Pass_Desc *pass) {
    int viewport_width  = 0;
    int viewport_height = 0;
    Color_Buffer_Binding color_binding = pass->render_target_bindings.color_bindings[0];
    if (color_binding.texture.valid) {
        viewport_width  = color_binding.texture.description.width;
        viewport_height = color_binding.texture.description.height;
    }
    else {
        get_swapchain_size(&viewport_width, &viewport_height);
    }
    ASSERT(viewport_width  != 0);
    ASSERT(viewport_height != 0);
    set_viewport(0, 0, viewport_width, viewport_height);

    assert(renderer_state.current_render_pass == nullptr);
    renderer_state.current_render_pass = pass;
    Pass_CBuffer pass_cbuffer = {};
    pass_cbuffer.screen_dimensions = v2((float)viewport_width, (float)viewport_height);
    pass_cbuffer.view_matrix = construct_view_matrix(pass->camera_position, pass->camera_orientation);
    pass_cbuffer.projection_matrix = pass->projection_matrix;
    pass_cbuffer.camera_position = pass->camera_position;
    update_buffer(renderer_state.pass_cbuffer_handle, &pass_cbuffer, sizeof(Pass_CBuffer));
    bind_constant_buffers(&renderer_state.pass_cbuffer_handle, 1, CBS_PASS);

    set_render_targets(pass->render_target_bindings);
}

void end_render_pass() {
    assert(renderer_state.current_render_pass != nullptr);
    renderer_state.current_render_pass = nullptr;
    unset_render_targets();
}

void draw_mesh(Buffer vertex_buffer, Buffer index_buffer, int num_vertices, int num_indices, Vector3 position, Vector3 scale, Quaternion orientation, Vector4 color) {
    Model_CBuffer model_cbuffer = {};
    model_cbuffer.model_matrix = construct_model_matrix(position, scale, orientation);
    model_cbuffer.model_color = color;

    update_buffer(renderer_state.model_cbuffer_handle, &model_cbuffer, sizeof(Model_CBuffer));
    bind_constant_buffers(&renderer_state.model_cbuffer_handle, 1, CBS_MODEL);

    u32 strides[1] = {sizeof(Vertex)};
    u32 offsets[1] = {0};
    bind_vertex_buffers(&vertex_buffer, 1, 0, strides, offsets);
    bind_index_buffer(index_buffer, 0);

    issue_draw_call(num_vertices, num_indices);
}

void draw_model(Model model, Vector3 position, Vector3 scale, Quaternion orientation, Vector4 color, Render_Options options, bool draw_transparency) {
    Foreach (mesh, model.meshes) {
        if (mesh->has_material) {
            if (mesh->material.has_transparency != draw_transparency) {
                continue;
            }
            ASSERT(mesh->material.cbuffer_handle);
            flush_pbr_material(mesh->material.cbuffer_handle, mesh->material, options);
        }
        draw_mesh(mesh->vertex_buffer, mesh->index_buffer, mesh->num_vertices, mesh->num_indices, position, scale, orientation, color);
    }
}

void draw_texture(Texture texture, Vector3 min, Vector3 max, float z_override) {
    Vertex ffverts[6];
    Array<Vertex> arr = make_array<Vertex>(ffverts, ARRAYSIZE(ffverts));
    Fixed_Function ff = {};
    ff_begin(&ff, &arr);
    bind_texture(texture, 0);
    Vector3 uvs[2] = {
        v3(0, 1, z_override),
        v3(1, 0, z_override),
    };
    ff_quad(&ff, min, max, v4(1, 1, 1, 1), uvs);
    ff_end(&ff);
    bind_texture({}, 0);
}

void draw_render_options_editor_window(Render_Options *render_options) {
    if (ImGui::Begin("Renderer")) {
        ImGui::Text("Render Mode");
        ImGui::RadioButton("Default",     reinterpret_cast<int *>(&render_options->debug_render_mode), (int)RM_DEFAULT); ImGui::SameLine();
        ImGui::RadioButton("Albedo",      reinterpret_cast<int *>(&render_options->debug_render_mode), (int)RM_ALBEDO);
        ImGui::RadioButton("Positions",   reinterpret_cast<int *>(&render_options->debug_render_mode), (int)RM_POSITIONS); ImGui::SameLine();
        ImGui::RadioButton("Normals",     reinterpret_cast<int *>(&render_options->debug_render_mode), (int)RM_NORMALS);
        ImGui::RadioButton("Metal/Rough", reinterpret_cast<int *>(&render_options->debug_render_mode), (int)RM_METALLIC_ROUGHNESS);

        ImGui::Checkbox("do albedo map",     &render_options->do_albedo_map);
        ImGui::Checkbox("do normal map",     &render_options->do_normal_map);
        ImGui::Checkbox("do metallic map",   &render_options->do_metallic_map);
        ImGui::Checkbox("do roughness map",  &render_options->do_roughness_map);
        ImGui::Checkbox("do emission map",   &render_options->do_emission_map);
        ImGui::Checkbox("do ao map",         &render_options->do_ao_map);

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



Blurrer make_blurrer(int width, int height, Vertex_Shader simple_vertex_shader, Pixel_Shader blur_pixel_shader, Pixel_Shader simple_textured_pixel_shader) {
    Blurrer blurrer = {};

    Texture_Description blur_ping_pong_desc = {};
    blur_ping_pong_desc.width  = width;
    blur_ping_pong_desc.height = height;
    blur_ping_pong_desc.type = TT_2D;
    blur_ping_pong_desc.wrap_mode = TWM_LINEAR_CLAMP;
    blur_ping_pong_desc.format = TF_R16G16B16A16_FLOAT;
    blur_ping_pong_desc.render_target = true;

    blurrer.ping_pong_color_buffers[0] = create_texture(blur_ping_pong_desc);
    blurrer.ping_pong_color_buffers[1] = create_texture(blur_ping_pong_desc);
    Texture_Description blur_ping_pong_depth_desc = blur_ping_pong_desc;
    blur_ping_pong_depth_desc.format = TF_DEPTH_STENCIL;
    blurrer.ping_pong_depth_buffer = create_texture(blur_ping_pong_depth_desc);

    blurrer.vertex_shader = simple_vertex_shader;
    blurrer.blur_pixel_shader = blur_pixel_shader;
    blurrer.simple_textured_pixel_shader = simple_textured_pixel_shader;
    blurrer.cbuffer_handle = create_buffer(BT_CONSTANT, nullptr, sizeof(Blur_CBuffer));

    return blurrer;
}

void destroy_blurrer(Blurrer blurrer) {
    destroy_texture(blurrer.ping_pong_color_buffers[0]);
    destroy_texture(blurrer.ping_pong_color_buffers[1]);
    destroy_texture(blurrer.ping_pong_depth_buffer);
    destroy_buffer(blurrer.cbuffer_handle);
}

void ensure_blurrer_texture_sizes(Blurrer *blurrer, int width, int height) {
    for (int i = 0; i < ARRAYSIZE(blurrer->ping_pong_color_buffers); i++) {
        ensure_texture_size(&blurrer->ping_pong_color_buffers[i], width, height);
    }
    ensure_texture_size(&blurrer->ping_pong_depth_buffer, width, height);
}

Texture do_blur(Blurrer *blurrer, Texture thing_to_blur, float radius, int num_iterations) {
    // initial copy pass
    {
        // todo(josh): this should just be a copy_texture()
        // todo(josh): this should just be a copy_texture()
        // todo(josh): this should just be a copy_texture()

        Render_Pass_Desc blur_pass = {};
        blur_pass.render_target_bindings.color_bindings[0] = {blurrer->ping_pong_color_buffers[1],   true, v4(0, 0, 0, 1)};
        blur_pass.render_target_bindings.depth_binding     = {blurrer->ping_pong_depth_buffer, true, 1};
        blur_pass.camera_orientation = quaternion_identity();
        blur_pass.projection_matrix = construct_orthographic_matrix(0, blurrer->ping_pong_color_buffers[1].description.width, 0, blurrer->ping_pong_color_buffers[1].description.height, -1, 1);
        begin_render_pass(&blur_pass);
        defer(end_render_pass());

        bind_shaders(blurrer->vertex_shader, blurrer->simple_textured_pixel_shader);
        draw_texture(
            thing_to_blur,
            v3(0, 0, 0),
            v3(blurrer->ping_pong_color_buffers[1].description.width, blurrer->ping_pong_color_buffers[1].description.height, 0));
    }


    Texture last_render_target = blurrer->ping_pong_color_buffers[1];
    bind_shaders(blurrer->vertex_shader, blurrer->blur_pixel_shader);
    for (int i = 0; i < (num_iterations * 2); i++) {
        Texture source_texture = blurrer->ping_pong_color_buffers[(i+1) % 2];
        last_render_target = blurrer->ping_pong_color_buffers[i % 2];

        Blur_CBuffer blur_cbuffer = {};
        blur_cbuffer.horizontal = i % 2;
        blur_cbuffer.buffer_dimensions = v2(source_texture.description.width, source_texture.description.height);
        blur_cbuffer.blur_radius = radius;
        update_buffer(blurrer->cbuffer_handle, &blur_cbuffer, sizeof(Blur_CBuffer));
        bind_constant_buffers(&blurrer->cbuffer_handle, 1, CBS_BLUR);

        Render_Pass_Desc blur_pass = {};
        blur_pass.render_target_bindings.color_bindings[0] = {last_render_target,     true, v4(0, 0, 0, 1)};
        blur_pass.render_target_bindings.depth_binding     = {blurrer->ping_pong_depth_buffer, true, 1};
        blur_pass.camera_orientation = quaternion_identity();
        blur_pass.projection_matrix = construct_orthographic_matrix(0, blurrer->ping_pong_color_buffers[1].description.width, 0, blurrer->ping_pong_color_buffers[1].description.height, -1, 1);
        begin_render_pass(&blur_pass);
        defer(end_render_pass());
        draw_texture(source_texture, v3(0, 0, 0), v3(last_render_target.description.width, last_render_target.description.height, 0));
    }

    return last_render_target;
}



void create_renderer3d(Renderer3D *out_renderer, Window *window) {
    // compile shaders
    out_renderer->vertex_shader                = compile_vertex_shader_from_file(L"vertex.hlsl");
    out_renderer->skybox_vertex_shader         = compile_vertex_shader_from_file(L"skybox_vertex.hlsl");
    out_renderer->pixel_shader                 = compile_pixel_shader_from_file(L"pixel.hlsl");
    out_renderer->simple_pixel_shader          = compile_pixel_shader_from_file(L"simple_pixel.hlsl");
    out_renderer->simple_textured_pixel_shader = compile_pixel_shader_from_file(L"simple_pixel_textured.hlsl");
    out_renderer->simple_pixel_3d_shader       = compile_pixel_shader_from_file(L"simple_pixel_3d.hlsl");
    out_renderer->text_pixel_shader            = compile_pixel_shader_from_file(L"text_pixel.hlsl");
    out_renderer->depth_pixel_shader           = compile_pixel_shader_from_file(L"depth_pixel.hlsl");
    out_renderer->blur_pixel_shader            = compile_pixel_shader_from_file(L"blur_pixel.hlsl");
    out_renderer->final_pixel_shader           = compile_pixel_shader_from_file(L"final_pixel.hlsl");
    out_renderer->skybox_pixel_shader          = compile_pixel_shader_from_file(L"skybox_pixel.hlsl");
    out_renderer->ssr_pixel_shader             = compile_pixel_shader_from_file(L"ssr_pixel.hlsl");

    // Make vertex format
    Vertex_Field vertex_fields[] = {
        {"SV_POSITION", "position",  offsetof(Vertex, position),  VFT_FLOAT3, VFST_PER_VERTEX},
        {"TEXCOORD",    "tex_coord", offsetof(Vertex, tex_coord), VFT_FLOAT3, VFST_PER_VERTEX},
        {"COLOR",       "color",     offsetof(Vertex, color),     VFT_FLOAT4, VFST_PER_VERTEX},
        {"NORMAL",      "normal",    offsetof(Vertex, normal),    VFT_FLOAT3, VFST_PER_VERTEX},
        {"TANGENT",     "tangent",   offsetof(Vertex, tangent),   VFT_FLOAT3, VFST_PER_VERTEX},
        {"BITANGENT",   "bitangent", offsetof(Vertex, bitangent), VFT_FLOAT3, VFST_PER_VERTEX},
    };
    out_renderer->default_vertex_format = create_vertex_format(vertex_fields, ARRAYSIZE(vertex_fields), out_renderer->vertex_shader);

    // make cube model
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
    out_renderer->cube_model = create_model(default_allocator());
    out_renderer->cube_model.meshes.append(cube_loaded_mesh);

    // create skybox
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
    out_renderer->skybox_texture = create_texture(skybox_desc);
    set_cubemap_textures(out_renderer->skybox_texture, skybox_faces);

    for (int idx = 0; idx < ARRAYSIZE(skybox_faces); idx++) {
        delete_texture_data(skybox_faces[idx]);
    }

    // make shadow map
    Texture_Description shadow_map_description = {};
    shadow_map_description.width = 2048;
    shadow_map_description.height = 2048;
    // todo(josh): this should just be one component per pixel
    shadow_map_description.format = TF_R16G16B16A16_FLOAT;
    shadow_map_description.wrap_mode = TWM_POINT_CLAMP;
    create_color_and_depth_buffers(shadow_map_description, &out_renderer->shadow_map_color_buffer, &out_renderer->shadow_map_depth_buffer);

    // make hdr buffer
    Texture_Description hdr_description = {};
    hdr_description.width = window->width;
    hdr_description.height = window->height;
    hdr_description.format = TF_R16G16B16A16_FLOAT;
    hdr_description.wrap_mode = TWM_LINEAR_CLAMP;
    create_color_and_depth_buffers(hdr_description, &out_renderer->hdr_color_buffer, &out_renderer->hdr_depth_buffer);

    // make gbuffer
    Texture_Description gbuffer_albedo_desc = {};
    gbuffer_albedo_desc.width = window->width;
    gbuffer_albedo_desc.height = window->height;
    gbuffer_albedo_desc.format = TF_R16G16B16A16_FLOAT; // todo(josh): do we really need this format? would r8g8b8a8_uint suffice?
    gbuffer_albedo_desc.render_target = true;
    out_renderer->gbuffer_albedo = create_texture(gbuffer_albedo_desc);

    Texture_Description gbuffer_normals_desc = {};
    gbuffer_normals_desc.width = window->width;
    gbuffer_normals_desc.height = window->height;
    gbuffer_normals_desc.format = TF_R16G16B16A16_FLOAT; // todo(josh): do we really need this format? would r8g8b8a8_uint suffice?
    gbuffer_normals_desc.render_target = true;
    out_renderer->gbuffer_normals = create_texture(gbuffer_normals_desc);

    Texture_Description gbuffer_positions_desc = {};
    gbuffer_positions_desc.width = window->width;
    gbuffer_positions_desc.height = window->height;
    gbuffer_positions_desc.format = TF_R16G16B16A16_FLOAT;
    gbuffer_positions_desc.render_target = true;
    out_renderer->gbuffer_positions = create_texture(gbuffer_positions_desc);

    Texture_Description gbuffer_metal_roughness_desc = {};
    gbuffer_metal_roughness_desc.width = window->width;
    gbuffer_metal_roughness_desc.height = window->height;
    gbuffer_metal_roughness_desc.format = TF_R8G8B8A8_UINT;
    gbuffer_metal_roughness_desc.render_target = true;
    out_renderer->gbuffer_metal_roughness = create_texture(gbuffer_metal_roughness_desc);

    // make bloom buffer
    Texture_Description bloom_desc = {};
    bloom_desc.width  = window->width;
    bloom_desc.height = window->height;
    bloom_desc.type = TT_2D;
    bloom_desc.format = TF_R16G16B16A16_FLOAT;
    bloom_desc.wrap_mode = TWM_LINEAR_CLAMP;
    bloom_desc.render_target = true;
    out_renderer->bloom_color_buffer = create_texture(bloom_desc);

    // make ssr buffer
    Texture_Description ssr_color_buffer_desc = {};
    ssr_color_buffer_desc.width  = window->width;
    ssr_color_buffer_desc.height = window->height;
    ssr_color_buffer_desc.type = TT_2D;
    ssr_color_buffer_desc.format = TF_R16G16B16A16_FLOAT;
    ssr_color_buffer_desc.wrap_mode = TWM_LINEAR_CLAMP;
    ssr_color_buffer_desc.render_target = true;
    out_renderer->ssr_color_buffer = create_texture(ssr_color_buffer_desc);

    // make final composite buffer
    Texture_Description final_composite_desc = {};
    final_composite_desc.width = window->width;
    final_composite_desc.height = window->height;
    final_composite_desc.format = TF_R16G16B16A16_FLOAT;
    final_composite_desc.wrap_mode = TWM_LINEAR_CLAMP;
    create_color_and_depth_buffers(final_composite_desc, &out_renderer->final_composite_color_buffer, &out_renderer->final_composite_depth_buffer);

    // make blurrer
    out_renderer->blurrer = make_blurrer(window->width / BLOOM_BUFFER_DOWNSCALE, window->height / BLOOM_BUFFER_DOWNSCALE, out_renderer->vertex_shader, out_renderer->blur_pixel_shader, out_renderer->simple_textured_pixel_shader);

    // make auto-exposure
    // todo(josh): do we need to do every single power of two?
    int auto_exposure_downsample_sizes[NUM_AUTO_EXPOSURE_DOWNSAMPLE_BUFFERS] = {
        512, 256, 128, 64, 32, 16, 8, 4, 2, 1,
    };

    Texture_Description auto_exposure_downsample_desc = {};
    auto_exposure_downsample_desc.wrap_mode = TWM_LINEAR_CLAMP;
    auto_exposure_downsample_desc.format = TF_R16G16B16A16_FLOAT;
    auto_exposure_downsample_desc.render_target = true;
    for (int i = 0; i < ARRAYSIZE(out_renderer->auto_exposure_downsample_buffers); i++) {
        Texture_Description desc = auto_exposure_downsample_desc;
        desc.width  = auto_exposure_downsample_sizes[i];
        desc.height = auto_exposure_downsample_sizes[i];
        out_renderer->auto_exposure_downsample_buffers[i] = create_texture(desc);
    }
    Texture_Description auto_exposure_cpu_read_buffer_desc = auto_exposure_downsample_desc;
    auto_exposure_cpu_read_buffer_desc.width  = 8;
    auto_exposure_cpu_read_buffer_desc.height = 8;
    auto_exposure_cpu_read_buffer_desc.render_target = false;
    auto_exposure_cpu_read_buffer_desc.cpu_read_target = true;
    out_renderer->auto_exposure_cpu_read_buffer = create_texture(auto_exposure_cpu_read_buffer_desc);

    out_renderer->current_exposure = 0.25;

    // make cbuffers
    out_renderer->lighting_cbuffer_handle = create_buffer(BT_CONSTANT, nullptr, sizeof(Lighting_CBuffer));
    out_renderer->ssr_cbuffer_handle      = create_buffer(BT_CONSTANT, nullptr, sizeof(SSR_CBuffer));
    out_renderer->final_cbuffer_handle    = create_buffer(BT_CONSTANT, nullptr, sizeof(Final_CBuffer));

    // make black and white textures
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
    out_renderer->white_texture = create_texture(white_texture_description);

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
    out_renderer->black_texture = create_texture(black_texture_description);
}

void destroy_renderer3d(Renderer3D *renderer) {
    // todo(josh): @leak
}

void render_scene(Renderer3D *renderer, Array<Draw_Command> render_queue, Vector3 camera_position, Quaternion camera_orientation, Render_Options render_options, Window *window, float time_since_startup, float dt) {
    ensure_swap_chain_size(window->width, window->height);

    ensure_texture_size(&renderer->hdr_color_buffer, window->width, window->height);
    ensure_texture_size(&renderer->hdr_depth_buffer, window->width, window->height);
    ensure_texture_size(&renderer->final_composite_color_buffer, window->width, window->height);
    ensure_texture_size(&renderer->final_composite_depth_buffer, window->width, window->height);
    // ensure_texture_size(&renderer->ssao_depth_color_buffer, window->width, window->height);
    ensure_texture_size(&renderer->gbuffer_albedo, window->width, window->height);
    ensure_texture_size(&renderer->gbuffer_positions, window->width, window->height);
    ensure_texture_size(&renderer->gbuffer_metal_roughness, window->width, window->height);
    ensure_texture_size(&renderer->gbuffer_normals, window->width, window->height);
    ensure_texture_size(&renderer->bloom_color_buffer, window->width, window->height);
    ensure_texture_size(&renderer->ssr_color_buffer, window->width, window->height);
    ensure_blurrer_texture_sizes(&renderer->blurrer, window->width / BLOOM_BUFFER_DOWNSCALE, window->height / BLOOM_BUFFER_DOWNSCALE);

    Fixed_Function ff = {};
    Array<Vertex> ff_vertices = make_array<Vertex>(default_allocator()); // @Alloc
    defer(ff_vertices.destroy());
    ff_begin(&ff, &ff_vertices);

    bind_vertex_format(renderer->default_vertex_format);
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
        shadow_pass.render_target_bindings.color_bindings[0] = {renderer->shadow_map_color_buffer, true, v4(0, 0, 0, 0)};
        shadow_pass.render_target_bindings.depth_binding     = {renderer->shadow_map_depth_buffer, true, 1};
        shadow_pass.camera_position = v3(0, 20, 0);
        shadow_pass.camera_orientation = sun_orientation;
        shadow_pass.projection_matrix = construct_orthographic_matrix(-20, 20, -20, 20, -100, 100);
        sun_transform = shadow_pass.projection_matrix * construct_view_matrix(shadow_pass.camera_position, shadow_pass.camera_orientation);
        begin_render_pass(&shadow_pass);
        defer(end_render_pass());
        bind_shaders(renderer->vertex_shader, renderer->depth_pixel_shader);

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
    bind_texture(renderer->skybox_texture, TS_PBR_SKYBOX);
    update_buffer(renderer->lighting_cbuffer_handle, &lighting, sizeof(Lighting_CBuffer));
    bind_constant_buffers(&renderer->lighting_cbuffer_handle, 1, CBS_LIGHTING);

    // draw scene
    {
        Render_Pass_Desc scene_pass_desc = {};
        scene_pass_desc.camera_position = camera_position;
        scene_pass_desc.camera_orientation = camera_orientation;
        scene_pass_desc.projection_matrix = construct_perspective_matrix(to_radians(60), (float)window->width / (float)window->height, 0.01, 1000);

        // draw scene to hdr buffer
        {
            Render_Pass_Desc scene_pass = scene_pass_desc;
            scene_pass.render_target_bindings.color_bindings[0] = {renderer->hdr_color_buffer,        true};
            scene_pass.render_target_bindings.color_bindings[1] = {renderer->bloom_color_buffer,      true};
            scene_pass.render_target_bindings.color_bindings[2] = {renderer->gbuffer_albedo,          true};
            scene_pass.render_target_bindings.color_bindings[3] = {renderer->gbuffer_positions,       true};
            scene_pass.render_target_bindings.color_bindings[4] = {renderer->gbuffer_normals,         true};
            scene_pass.render_target_bindings.color_bindings[5] = {renderer->gbuffer_metal_roughness, true};
            scene_pass.render_target_bindings.depth_binding     = {renderer->hdr_depth_buffer,        true, 1};
            begin_render_pass(&scene_pass);
            defer(end_render_pass());
            bind_texture(renderer->shadow_map_color_buffer, TS_PBR_SHADOW_MAP);
            defer(bind_texture({}, TS_PBR_SHADOW_MAP));
            bind_shaders(renderer->vertex_shader, renderer->pixel_shader);
            Foreach (command, render_queue) {
                draw_model(command->model, command->position, command->scale, command->orientation, command->color, render_options, false);
                draw_model(command->model, command->position, command->scale, command->orientation, command->color, render_options, true);
            }

            // skybox
            bind_shaders(renderer->skybox_vertex_shader, renderer->skybox_pixel_shader);
            bind_texture(renderer->skybox_texture, TS_PBR_ALBEDO);

            set_backface_cull(false);
            draw_model(renderer->cube_model, camera_position, v3(1, 1, 1), quaternion_identity(), skybox_color, render_options, false);
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
            update_buffer(renderer->ssr_cbuffer_handle, &ssr_cbuffer, sizeof(SSR_CBuffer));
            bind_constant_buffers(&renderer->ssr_cbuffer_handle, 1, CBS_SSR);

            Render_Pass_Desc ssr_pass = {};
            ssr_pass.render_target_bindings.color_bindings[0] = {renderer->ssr_color_buffer, true};
            ssr_pass.render_target_bindings.depth_binding     = {{},               true, 1}; // todo(josh): we really shouldn't be binding a depth buffer here
            ssr_pass.camera_position = v3(0, 0, 0);
            ssr_pass.camera_orientation = quaternion_identity();
            ssr_pass.projection_matrix = construct_orthographic_matrix(0, window->width, 0, window->height, -1, 1);
            begin_render_pass(&ssr_pass);
            defer(end_render_pass());

            bind_texture(renderer->gbuffer_albedo, TS_SSR_ALBEDO_MAP);
            defer(bind_texture({}, TS_SSR_ALBEDO_MAP));
            bind_texture(renderer->gbuffer_normals, TS_SSR_NORMAL_MAP);
            defer(bind_texture({}, TS_SSR_NORMAL_MAP));
            bind_texture(renderer->gbuffer_positions, TS_SSR_POSITIONS_MAP);
            defer(bind_texture({}, TS_SSR_POSITIONS_MAP));
            bind_texture(renderer->gbuffer_metal_roughness, TS_SSR_METAL_ROUGHNESS_MAP);
            defer(bind_texture({}, TS_SSR_POSITIONS_MAP));

            bind_shaders(renderer->vertex_shader, renderer->ssr_pixel_shader);
            draw_texture(renderer->hdr_color_buffer, v3(0, 0, 0), v3(window->width, window->height, 0));
        }
    }

    Texture last_bloom_blur_render_target = do_blur(&renderer->blurrer, renderer->bloom_color_buffer, render_options.bloom_radius, render_options.bloom_iterations);
    // Texture last_ssr_blur_render_target = do_blur(&blurrer, ssr_color_buffer, 2, 1);

    {
        u64 *pixels = (u64 *)map_texture(&renderer->auto_exposure_cpu_read_buffer);
        defer(unmap_texture(&renderer->auto_exposure_cpu_read_buffer));

        int pixel_size = get_texture_format_info(renderer->auto_exposure_cpu_read_buffer.description.format).pixel_size_in_bytes;
        assert(pixel_size == sizeof(u64));
        int pixels_length_in_bytes = renderer->auto_exposure_cpu_read_buffer.description.width * renderer->auto_exposure_cpu_read_buffer.description.height * pixel_size;
        int pixels_length_in_u64 = pixels_length_in_bytes / 8;
        u64 middle_pixel = pixels[4 + (renderer->auto_exposure_cpu_read_buffer.description.width * 4)];
        u16 middle_pixel_r = (u16)(middle_pixel >> 0);
        u16 middle_pixel_g = (u16)(middle_pixel >> 16);
        u16 middle_pixel_b = (u16)(middle_pixel >> 32);
        u32 ru32 = half_to_float(middle_pixel_r);
        float r = *((float *)(&ru32));
        u32 gu32 = half_to_float(middle_pixel_g);
        float g = *((float *)(&gu32));
        u32 bu32 = half_to_float(middle_pixel_b);
        float b = *((float *)(&bu32));

        Vector3 color = v3(r*r, g*g, b*b);
        float brightness = dot(color, v3(0.2, 0.7, 0.1)); // todo(josh): @CorrectBrightness (0.2, 0.7, 0.1) are not exact. martijn: "if you want the exact ones, look at wikipedia at the Y component of the RGB primaries of the sRGB color space"
        float exposure_this_frame = render_options.exposure_modifier / (brightness + 1e-3);

        if (fabsf(renderer->current_exposure - exposure_this_frame) > 0.1) {
            const float EXPOSURE_SPEED = 0.5;
            if (renderer->current_exposure < exposure_this_frame) {
                renderer->current_exposure += EXPOSURE_SPEED * dt;
                if (renderer->current_exposure > exposure_this_frame) {
                    renderer->current_exposure = exposure_this_frame;
                }
            }
            else if (renderer->current_exposure > exposure_this_frame) {
                renderer->current_exposure -= EXPOSURE_SPEED * dt;
                if (renderer->current_exposure < exposure_this_frame) {
                    renderer->current_exposure = exposure_this_frame;
                }
            }
        }
        else {
            renderer->current_exposure = lerp(renderer->current_exposure, exposure_this_frame, 1 * dt);
        }
    }

    // make the final composite
    {
        Render_Pass_Desc screen_pass = {};
        screen_pass.render_target_bindings.color_bindings[0] = {renderer->final_composite_color_buffer, true, v4(0, 0, 0, 0)};
        screen_pass.render_target_bindings.depth_binding     = {renderer->final_composite_depth_buffer, true, 1};
        screen_pass.camera_position = v3(0, 0, 0);
        screen_pass.camera_orientation = quaternion_identity();
        screen_pass.projection_matrix = construct_orthographic_matrix(0, window->width, 0, window->height, -1, 1);
        begin_render_pass(&screen_pass);
        defer(end_render_pass());

        bind_texture(last_bloom_blur_render_target, TS_FINAL_BLOOM_MAP);
        defer(bind_texture({}, TS_FINAL_BLOOM_MAP));
        bind_texture(renderer->ssr_color_buffer, TS_FINAL_SSR_MAP);
        defer(bind_texture({}, TS_FINAL_SSR_MAP));

        bind_shaders(renderer->vertex_shader, renderer->final_pixel_shader);

        Final_CBuffer final_cbuffer = {};
        final_cbuffer.exposure = renderer->current_exposure;
        update_buffer(renderer->final_cbuffer_handle, &final_cbuffer, sizeof(Final_CBuffer));
        bind_constant_buffers(&renderer->final_cbuffer_handle, 1, CBS_FINAL);
        draw_texture(renderer->hdr_color_buffer, v3(0, 0, 0), v3(window->width, window->height, 0));
    }

    // calculate exposure brightness for next frame
    {
        set_alpha_blend(false);
        defer(set_alpha_blend(true));
        bind_shaders(renderer->vertex_shader, renderer->simple_textured_pixel_shader);
        Texture thing_to_draw = renderer->final_composite_color_buffer;
        for (int i = 0; i < ARRAYSIZE(renderer->auto_exposure_downsample_buffers); i++) {
            Render_Pass_Desc screen_pass = {};
            screen_pass.render_target_bindings.color_bindings[0] = {renderer->auto_exposure_downsample_buffers[i], true, v4(0, 0, 0, 0)};
            screen_pass.render_target_bindings.depth_binding     = {{}, true, 1};
            screen_pass.camera_position = v3(0, 0, 0);
            screen_pass.camera_orientation = quaternion_identity();
            screen_pass.projection_matrix = construct_orthographic_matrix(0, thing_to_draw.description.width, 0, thing_to_draw.description.height, -1, 1);
            begin_render_pass(&screen_pass);
            defer(end_render_pass());
            draw_texture(thing_to_draw, v3(0, 0, 0), v3(thing_to_draw.description.width, thing_to_draw.description.height, 0));
            thing_to_draw = renderer->auto_exposure_downsample_buffers[i];
        }
        copy_texture(renderer->auto_exposure_cpu_read_buffer, renderer->auto_exposure_downsample_buffers[6]);
    }

    // actually draw to the screen
    {
        Render_Pass_Desc screen_pass = {};
        screen_pass.render_target_bindings.color_bindings[0] = {{}, true, v4(0.39, 0.58, 0.93, 1.0f)};
        screen_pass.render_target_bindings.depth_binding     = {{}, true, 1};
        screen_pass.camera_position = v3(0, 0, 0);
        screen_pass.camera_orientation = quaternion_identity();
        screen_pass.projection_matrix = construct_orthographic_matrix(0, window->width, 0, window->height, -1, 1);
        begin_render_pass(&screen_pass);
        defer(end_render_pass());

        bind_shaders(renderer->vertex_shader, renderer->simple_textured_pixel_shader);

        switch (render_options.debug_render_mode) {
            case RM_DEFAULT:            draw_texture(renderer->final_composite_color_buffer, v3(0, 0, 0),   v3(window->width, window->height, 0)); break;
            case RM_ALBEDO:             draw_texture(renderer->gbuffer_albedo,               v3(0, 0, 0),   v3(window->width, window->height, 0)); break;
            case RM_POSITIONS:          draw_texture(renderer->gbuffer_positions,            v3(0, 0, 0),   v3(window->width, window->height, 0)); break;
            case RM_NORMALS:            draw_texture(renderer->gbuffer_normals,              v3(0, 0, 0),   v3(window->width, window->height, 0)); break;
            case RM_METALLIC_ROUGHNESS: draw_texture(renderer->gbuffer_metal_roughness,      v3(0, 0, 0),   v3(window->width, window->height, 0)); break;
        }

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

        Vector3 text_pos = v3(10, window->height-roboto_mono.pixel_height, 0);
        const float text_size = 1;
        ff_text(&ff, "1. do_albedo_map",     roboto_mono, v4(1, 1, 1, render_options.do_albedo_map     ? 1.0 : 0.2), text_pos, text_size); text_pos.y -= roboto_mono.pixel_height * text_size;
        ff_text(&ff, "2. do_normal_map",     roboto_mono, v4(1, 1, 1, render_options.do_normal_map     ? 1.0 : 0.2), text_pos, text_size); text_pos.y -= roboto_mono.pixel_height * text_size;
        ff_text(&ff, "3. do_metallic_map",   roboto_mono, v4(1, 1, 1, render_options.do_metallic_map   ? 1.0 : 0.2), text_pos, text_size); text_pos.y -= roboto_mono.pixel_height * text_size;
        ff_text(&ff, "4. do_roughness_map",  roboto_mono, v4(1, 1, 1, render_options.do_roughness_map  ? 1.0 : 0.2), text_pos, text_size); text_pos.y -= roboto_mono.pixel_height * text_size;
        ff_text(&ff, "5. do_emission_map",   roboto_mono, v4(1, 1, 1, render_options.do_emission_map   ? 1.0 : 0.2), text_pos, text_size); text_pos.y -= roboto_mono.pixel_height * text_size;
        ff_text(&ff, "6. do_ao_map",         roboto_mono, v4(1, 1, 1, render_options.do_ao_map         ? 1.0 : 0.2), text_pos, text_size); text_pos.y -= roboto_mono.pixel_height * text_size;
        ff_end(&ff);
        */
    }
}



void ff_begin(Fixed_Function *ff, Array<Vertex> *array) {
    ff->array = array;
}

void ff_flush(Fixed_Function *ff) {
    if (ff->array->count == 0) {
        return;
    }

    Buffer vertex_buffer = create_buffer(BT_VERTEX, ff->array->data, sizeof((*ff->array)[0]) * ff->array->count);
    draw_mesh(vertex_buffer, nullptr, ff->array->count, 0, v3(0, 0, 0), v3(1, 1, 1), quaternion_identity(), v4(1, 1, 1, 1));
    destroy_buffer(vertex_buffer);
    ff->array->clear();
    ff->current_vertex = nullptr;
}

void ff_end(Fixed_Function *ff) {
    ff_flush(ff);
}

void ff_vertex(Fixed_Function *ff, Vector3 position) {
    Vertex v = {};
    v.position = position;
    ff->current_vertex = ff->array->append(v);
}

void ff_tex_coord(Fixed_Function *ff, Vector3 tex_coord) {
    ASSERT(ff->current_vertex != nullptr);
    ff->current_vertex->tex_coord = tex_coord;
}

void ff_color(Fixed_Function *ff, Vector4 color) {
    ASSERT(ff->current_vertex != nullptr);
    ff->current_vertex->color = color;
}

void ff_quad(Fixed_Function *ff, Vector3 min, Vector3 max, Vector4 color, Vector3 uv_overrides[2]) {
    Vector3 uvs[2] = {
        v3(0, 1, 0),
        v3(1, 0, 0),
    };
    if (uv_overrides) {
        uvs[0] = uv_overrides[0];
        uvs[1] = uv_overrides[1];
    }
    ff_vertex(ff, v3(min.x, min.y, 0)); ff_tex_coord(ff, v3(uvs[0].x, uvs[0].y, uvs[0].z)); ff_color(ff, color);
    ff_vertex(ff, v3(min.x, max.y, 0)); ff_tex_coord(ff, v3(uvs[0].x, uvs[1].y, uvs[0].z)); ff_color(ff, color);
    ff_vertex(ff, v3(max.x, max.y, 0)); ff_tex_coord(ff, v3(uvs[1].x, uvs[1].y, uvs[0].z)); ff_color(ff, color);
    ff_vertex(ff, v3(max.x, max.y, 0)); ff_tex_coord(ff, v3(uvs[1].x, uvs[1].y, uvs[0].z)); ff_color(ff, color);
    ff_vertex(ff, v3(max.x, min.y, 0)); ff_tex_coord(ff, v3(uvs[1].x, uvs[0].y, uvs[0].z)); ff_color(ff, color);
    ff_vertex(ff, v3(min.x, min.y, 0)); ff_tex_coord(ff, v3(uvs[0].x, uvs[0].y, uvs[0].z)); ff_color(ff, color);
}

void ff_line(Fixed_Function *ff, Vector3 a, Vector3 b, Vector4 color) {
    ff_vertex(ff, a); ff_color(ff, color);
    ff_vertex(ff, b); ff_color(ff, color);
}

void ff_line_circle(Fixed_Function *ff, Vector3 position, float radius, Vector3 normal, Vector4 color, float resolution) {
    Vector3 perp  = arbitrary_perpendicular(normal);
    Vector3 perp2 = cross(normal, perp);
    assert(length(normal) == 1);
    assert(length(perp)   == 1);
    assert(length(perp2)  == 1);
    Vector3 columns[3] = {
        perp,
        perp2,
        normal,
    };
    Matrix3 transform = m3(columns);
    int last_theta = 0;
    for (int theta = last_theta + resolution; theta <= 360; theta += resolution) {
        float last_x = cos(to_radians((float)last_theta)) - sin(to_radians((float)last_theta)) * radius;
        float last_y = sin(to_radians((float)last_theta)) + cos(to_radians((float)last_theta)) * radius;

        float x = cos(to_radians((float)theta)) - sin(to_radians((float)theta)) * radius;
        float y = sin(to_radians((float)theta)) + cos(to_radians((float)theta)) * radius;

        Vector3 v0 = transform * v3(last_x, last_y, 0);
        Vector3 v1 = transform * v3(x, y, 0);
        ff_line(ff, position + v0, position + v1, color);
        last_theta = theta;
    }
}

void ff_text(Fixed_Function *ff, char *str, Font font, Vector4 color, Vector3 start_pos, float size) {
    Vector3 position = {};
    for (char *c = str; *c != '\0'; c++) {
        if (*c == '\n') {
            position.x = 0;
            position.y += font.pixel_height * size;
            continue;
        }

        stbtt_aligned_quad quad;
        stbtt_GetBakedQuad(font.chars, font.dim, font.dim, *c, &position.x, &position.y, &quad, 1);//1=opengl & d3d10+,0=d3d9
        float x0 = start_pos.x + quad.x0 * size;
        float y0 = start_pos.y + quad.y0 * size;
        float x1 = start_pos.x + quad.x1 * size;
        float y1 = start_pos.y + quad.y1 * size;
        float miny = start_pos.y - (y1 - start_pos.y);
        float character_height = y1 - y0;
        Vector3 uvs[2] = {
            v3(quad.s0, quad.t1, 0),
            v3(quad.s1, quad.t0, 0),
        };
        ff_quad(ff, v3(x0, miny, start_pos.z), v3(x1, miny + character_height, start_pos.z), color, uvs);
    }
}
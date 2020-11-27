#include "renderer.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

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
    material_cbuffer.visualize_normals = options.visualize_normals;

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
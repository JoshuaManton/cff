#include "renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

struct Renderer_State {
    Render_Pass_Desc *current_render_pass;
    Buffer pass_cbuffer_handle;
    Buffer model_cbuffer_handle;
    Buffer pbr_material_cbuffer_handle;
};

Renderer_State renderer_state;

void init_renderer(Window *window) {
    renderer_state.pass_cbuffer_handle         = create_buffer(BT_CONSTANT, nullptr, sizeof(Pass_CBuffer));
    renderer_state.model_cbuffer_handle        = create_buffer(BT_CONSTANT, nullptr, sizeof(Model_CBuffer));
    renderer_state.pbr_material_cbuffer_handle = create_buffer(BT_CONSTANT, nullptr, sizeof(PBR_Material_CBuffer));
}

Texture create_texture_from_file(char *filename, Texture_Format format, Texture_Wrap_Mode wrap_mode) {
    int width;
    int height;
    byte *color_data = load_texture_data_from_file(filename, &width, &height);
    if (!color_data) {
        printf("create_texture_from_file() couldn't find file: %s\n", filename);
        return {};
    }
    defer(delete_texture_data(color_data));

    Texture_Description texture_description = {};
    texture_description.width = width;
    texture_description.height = height;
    texture_description.color_data = color_data;
    texture_description.format = format;
    texture_description.wrap_mode = wrap_mode;
    texture_description.type = TT_2D;
    Texture texture = create_texture(texture_description);
    return texture;
}

byte *load_texture_data_from_file(char *filename, int *width, int *height) {
    int filedata_len;
    char *filedata = read_entire_file(filename, &filedata_len);
    if (!filedata) {
        printf("load_texture_data_from_file() couldn't find file: %s\n", filename);
        return {};
    }
    assert(filedata != nullptr);
    defer(free(filedata));
    // stbi_set_flip_vertically_on_load(1);
    int n;
    byte *color_data = stbi_load_from_memory((byte *)filedata, filedata_len, width, height, &n, 4);
    assert(color_data);
    return color_data;
}

void delete_texture_data(byte *data) {
    stbi_image_free(data);
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
    assert(renderer_state.current_render_pass == nullptr);
    renderer_state.current_render_pass = pass;
    Pass_CBuffer pass_cbuffer = {};
    pass_cbuffer.view_matrix = construct_view_matrix(pass->camera_position, pass->camera_orientation);
    pass_cbuffer.projection_matrix = pass->projection_matrix;
    pass_cbuffer.camera_position = pass->camera_position;
    update_buffer(renderer_state.pass_cbuffer_handle, &pass_cbuffer, sizeof(Pass_CBuffer));
    bind_constant_buffers(&renderer_state.pass_cbuffer_handle, 1, CBS_PASS);
}

void end_render_pass() {
    assert(renderer_state.current_render_pass != nullptr);
    renderer_state.current_render_pass = nullptr;
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

void draw_meshes(Array<Loaded_Mesh> meshes, Vector3 position, Vector3 scale, Quaternion orientation, Vector4 color, Render_Options options, bool draw_transparency) {
    Foreach (mesh, meshes) {
        if (mesh->has_material) {
            if (mesh->material.has_transparency != draw_transparency) {
                continue;
            }
            flush_pbr_material(renderer_state.pbr_material_cbuffer_handle, mesh->material, options);
        }
        draw_mesh(mesh->vertex_buffer, mesh->index_buffer, mesh->num_vertices, mesh->num_indices, position, scale, orientation, color);
    }
}

void draw_texture(Texture texture, Vector3 min, Vector3 max, float z_override) {
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

void ff_begin(Fixed_Function *ff, Vertex *buffer, int max_vertices) {
    ff->num_vertices = 0;
    ff->vertices = buffer;
    ff->max_vertices = max_vertices;
    ff->vertex_buffer = create_buffer(BT_VERTEX, nullptr, sizeof(ff->vertices[0]) * max_vertices);
}

void ff_flush(Fixed_Function *ff) {
    assert(ff->num_vertices <= ff->max_vertices);
    if (ff->num_vertices == 0) {
        return;
    }

    update_buffer(ff->vertex_buffer, ff->vertices, sizeof(ff->vertices[0]) * ff->num_vertices);
    draw_mesh(ff->vertex_buffer, nullptr, ff->num_vertices, 0, v3(0, 0, 0), v3(1, 1, 1), quaternion_identity(), v4(1, 1, 1, 1));
    ff->num_vertices = 0;
}

void ff_end(Fixed_Function *ff) {
    ff_flush(ff);
    destroy_buffer(ff->vertex_buffer);
}

void ff_vertex(Fixed_Function *ff, Vector3 position) {
    assert(ff->num_vertices < ff->max_vertices);
    ff->vertices[ff->num_vertices].position = v3(position.x, position.y, position.z);
}

void ff_tex_coord(Fixed_Function *ff, Vector3 tex_coord) {
    assert(ff->num_vertices < ff->max_vertices);
    ff->vertices[ff->num_vertices].tex_coord = tex_coord;
}

void ff_color(Fixed_Function *ff, Vector4 color) {
    assert(ff->num_vertices < ff->max_vertices);
    ff->vertices[ff->num_vertices].color = color;
}

void ff_next(Fixed_Function *ff) {
    ff->num_vertices += 1;
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
    ff_vertex(ff, v3(min.x, min.y, 0)); ff_tex_coord(ff, v3(uvs[0].x, uvs[0].y, uvs[0].z)); ff_color(ff, color); ff_next(ff);
    ff_vertex(ff, v3(min.x, max.y, 0)); ff_tex_coord(ff, v3(uvs[0].x, uvs[1].y, uvs[0].z)); ff_color(ff, color); ff_next(ff);
    ff_vertex(ff, v3(max.x, max.y, 0)); ff_tex_coord(ff, v3(uvs[1].x, uvs[1].y, uvs[0].z)); ff_color(ff, color); ff_next(ff);
    ff_vertex(ff, v3(max.x, max.y, 0)); ff_tex_coord(ff, v3(uvs[1].x, uvs[1].y, uvs[0].z)); ff_color(ff, color); ff_next(ff);
    ff_vertex(ff, v3(max.x, min.y, 0)); ff_tex_coord(ff, v3(uvs[1].x, uvs[0].y, uvs[0].z)); ff_color(ff, color); ff_next(ff);
    ff_vertex(ff, v3(min.x, min.y, 0)); ff_tex_coord(ff, v3(uvs[0].x, uvs[0].y, uvs[0].z)); ff_color(ff, color); ff_next(ff);
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
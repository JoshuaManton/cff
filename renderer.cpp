#include "renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

struct Renderer_State {
    Buffer pass_cbuffer_handle;
    Buffer model_cbuffer_handle;
};

Renderer_State renderer_state;

void init_renderer(Window *window) {
    // Make vertex format
    Vertex_Field fields[] = {
        {"SV_POSITION", "position",  offsetof(Vertex, position),  VFT_FLOAT3, VFST_PER_VERTEX},
        {"TEXCOORD",    "tex_coord", offsetof(Vertex, tex_coord), VFT_FLOAT3, VFST_PER_VERTEX},
        {"COLOR",       "color",     offsetof(Vertex, color),     VFT_FLOAT4, VFST_PER_VERTEX},
    };
    renderer_state.pass_cbuffer_handle  = create_buffer(BT_CONSTANT, nullptr, sizeof(Pass_CBuffer));
    renderer_state.model_cbuffer_handle = create_buffer(BT_CONSTANT, nullptr, sizeof(Model_CBuffer));
}

Texture load_texture_from_file(char *filename, Texture_Format format) {
    int filedata_len;
    char *filedata = read_entire_file(filename, &filedata_len);
    defer(free(filedata));
    // stbi_set_flip_vertically_on_load(1);
    int x, y, n;
    byte *color_data = stbi_load_from_memory((byte *)filedata, filedata_len, &x, &y, &n, 4);
    assert(color_data);
    defer(stbi_image_free(color_data));

    Texture_Description texture_description = {};
    texture_description.width = x;
    texture_description.height = y;
    texture_description.color_data = color_data;
    texture_description.format = format;
    texture_description.type = TT_2D;
    Texture texture = create_texture(texture_description);
    return texture;
}

Font load_font_from_file(char *filename, float size) {
    int ttf_data_len;
    unsigned char *ttf_data = (unsigned char *)read_entire_file(filename, &ttf_data_len);
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
    desc.color_data = pixels;
    font.texture = create_texture(desc);
    return font;
}

void destroy_font(Font font) {
    destroy_texture(font.texture);
}

void begin_render_pass(Render_Pass_Desc *pass) {
    Pass_CBuffer pass_cbuffer = {};
    pass_cbuffer.view_matrix = view_matrix(pass->camera_position, pass->camera_orientation);
    pass_cbuffer.projection_matrix = pass->projection_matrix;
    pass_cbuffer.camera_position = pass->camera_position;
    update_buffer(renderer_state.pass_cbuffer_handle, &pass_cbuffer, sizeof(Pass_CBuffer));
    bind_constant_buffers(&renderer_state.pass_cbuffer_handle, 1, CBS_PASS);
}

void draw_meshes(Array<Loaded_Mesh> meshes, Vector3 position, Vector3 scale, Quaternion orientation, Render_Options options) {
    Foreach(mesh, meshes) {
        Model_CBuffer model_cbuffer = {};
        model_cbuffer.model_matrix = model_matrix(position, scale, orientation);
        model_cbuffer.visualize_normals = options.visualize_normals;
        if (mesh->has_material) {
            model_cbuffer.ambient   = mesh->material.ambient;
            model_cbuffer.metallic  = mesh->material.metallic;
            model_cbuffer.roughness = mesh->material.roughness;

            if (mesh->material.albedo_map.handle && options.do_albedo_map) {
                bind_textures(&mesh->material.albedo_map, 1, TS_ALBEDO);
                model_cbuffer.has_albedo_map = 1;
            }
            if (mesh->material.normal_map.handle && options.do_normal_map) {
                bind_textures(&mesh->material.normal_map, 1, TS_NORMAL);
                model_cbuffer.has_normal_map = 1;
            }
            if (mesh->material.metallic_map.handle && options.do_metallic_map) {
                bind_textures(&mesh->material.metallic_map, 1, TS_METALLIC);
                model_cbuffer.has_metallic_map = 1;
            }
            if (mesh->material.roughness_map.handle && options.do_roughness_map) {
                bind_textures(&mesh->material.roughness_map, 1, TS_ROUGHNESS);
                model_cbuffer.has_roughness_map = 1;
            }
            if (mesh->material.emission_map.handle && options.do_emission_map) {
                bind_textures(&mesh->material.emission_map, 1, TS_EMISSION);
                model_cbuffer.has_emission_map = 1;
            }
            if (mesh->material.ao_map.handle && options.do_ao_map) {
                bind_textures(&mesh->material.ao_map, 1, TS_AO);
                model_cbuffer.has_ao_map = 1;
            }
        }

        update_buffer(renderer_state.model_cbuffer_handle, &model_cbuffer, sizeof(Model_CBuffer));
        bind_constant_buffers(&renderer_state.model_cbuffer_handle, 1, CBS_MODEL);

        u32 strides[1] = {sizeof(Vertex)};
        u32 offsets[1] = {0};
        bind_vertex_buffers(&mesh->vertex_buffer, 1, 0, strides, offsets);
        bind_index_buffer(mesh->index_buffer, 0);

        issue_draw_call(mesh->num_vertices, mesh->num_indices);
    }
}

void ff_begin(Fixed_Function *ff, Vertex *buffer, int max_vertices, Texture texture, Vertex_Shader vertex_shader, Pixel_Shader pixel_shader) {
    ff->num_vertices = 0;
    ff->vertices = buffer;
    ff->max_vertices = max_vertices;
    ff->texture = texture;
    ff->vertex_shader = vertex_shader;
    ff->pixel_shader = pixel_shader;
}

void ff_end(Fixed_Function *ff) {
    assert(ff->num_vertices < ff->max_vertices);
    // todo(josh): should this create() go in ff_begin?
    Buffer vertex_buffer = create_buffer(BT_VERTEX, ff->vertices, sizeof(ff->vertices[0]) * ff->num_vertices);
    u32 strides[1] = { sizeof(ff->vertices[0]) };
    u32 offsets[1] = { 0 };
    bind_vertex_buffers(&vertex_buffer, 1, 0, strides, offsets);

    Model_CBuffer model_cbuffer = {};
    model_cbuffer.model_matrix = model_matrix(v3(0, 0, 0), v3(1, 1, 1), quaternion_identity());
    if (ff->texture.handle) {
        bind_textures(&ff->texture, 1, TS_ALBEDO);
        model_cbuffer.has_albedo_map = 1;
    }
    update_buffer(renderer_state.model_cbuffer_handle, &model_cbuffer, sizeof(Model_CBuffer));
    bind_constant_buffers(&renderer_state.model_cbuffer_handle, 1, CBS_MODEL);

    bind_shaders(ff->vertex_shader, ff->pixel_shader);

    issue_draw_call(ff->num_vertices, 0);
    destroy_buffer(vertex_buffer);
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

void ff_quad(Fixed_Function *ff, Vector3 min, Vector3 max, Vector4 color, Vector2 uv_overrides[2]) {
    ff_vertex(ff, v3(min.x, min.y, 0)); if (uv_overrides) { ff_tex_coord(ff, v3(uv_overrides[0].x, uv_overrides[0].y, 0)); } ff_color(ff, color); ff_next(ff);
    ff_vertex(ff, v3(min.x, max.y, 0)); if (uv_overrides) { ff_tex_coord(ff, v3(uv_overrides[0].x, uv_overrides[1].y, 0)); } ff_color(ff, color); ff_next(ff);
    ff_vertex(ff, v3(max.x, max.y, 0)); if (uv_overrides) { ff_tex_coord(ff, v3(uv_overrides[1].x, uv_overrides[1].y, 0)); } ff_color(ff, color); ff_next(ff);
    ff_vertex(ff, v3(max.x, max.y, 0)); if (uv_overrides) { ff_tex_coord(ff, v3(uv_overrides[1].x, uv_overrides[1].y, 0)); } ff_color(ff, color); ff_next(ff);
    ff_vertex(ff, v3(max.x, min.y, 0)); if (uv_overrides) { ff_tex_coord(ff, v3(uv_overrides[1].x, uv_overrides[0].y, 0)); } ff_color(ff, color); ff_next(ff);
    ff_vertex(ff, v3(min.x, min.y, 0)); if (uv_overrides) { ff_tex_coord(ff, v3(uv_overrides[0].x, uv_overrides[0].y, 0)); } ff_color(ff, color); ff_next(ff);
}

void ff_text(Fixed_Function *ff, char *str, Font font, Vector4 color, Vector3 start_pos, float size) {
    Vector3 position = {};
    for (char *c = str; *c != '\0'; c++) {
        stbtt_aligned_quad quad;
        stbtt_GetBakedQuad(font.chars, font.dim, font.dim, *c, &position.x, &position.y, &quad, 1);//1=opengl & d3d10+,0=d3d9
        float x0 = start_pos.x + quad.x0 * size;
        float y0 = start_pos.y + quad.y0 * size;
        float x1 = start_pos.x + quad.x1 * size;
        float y1 = start_pos.y + quad.y1 * size;
        float miny = start_pos.y - (y1 - start_pos.y);
        float character_height = y1 - y0;
        Vector2 uvs[2] = {
            v2(quad.s0, quad.t1),
            v2(quad.s1, quad.t0),
        };
        ff_quad(ff, v3(x0, miny, start_pos.z), v3(x1, miny + character_height, start_pos.z), color, uvs);
    }
}
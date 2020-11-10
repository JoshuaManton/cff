#include "renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct Renderer_State {
    Buffer pass_cbuffer_handle;
    Buffer model_cbuffer_handle;

    Render_Pass_Desc *current_render_pass;
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

void begin_render_pass(Render_Pass_Desc *pass) {
    renderer_state.current_render_pass = pass;
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
        if (mesh->has_material) {
            if (mesh->material.albedo_map.handle && options.do_albedo) {
                bind_textures(&mesh->material.albedo_map, 1, TS_ALBEDO);
                model_cbuffer.has_albedo_map = 1;
            }
            if (mesh->material.normal_map.handle && options.do_normal) {
                bind_textures(&mesh->material.normal_map, 1, TS_NORMAL);
                model_cbuffer.has_normal_map = 1;
            }
            if (mesh->material.metallic_map.handle && options.do_metallic) {
                bind_textures(&mesh->material.metallic_map, 1, TS_METALLIC);
                model_cbuffer.has_metallic_map = 1;
            }
            if (mesh->material.roughness_map.handle && options.do_roughness) {
                bind_textures(&mesh->material.roughness_map, 1, TS_ROUGHNESS);
                model_cbuffer.has_roughness_map = 1;
            }
            if (mesh->material.emission_map.handle && options.do_emission) {
                bind_textures(&mesh->material.emission_map, 1, TS_EMISSION);
                model_cbuffer.has_emission_map = 1;
            }
            if (mesh->material.ao_map.handle && options.do_ao) {
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

void ff_begin(Fixed_Function *ff, Vertex *buffer, int max_vertices) {
    ff->num_vertices = 0;
    ff->vertices = buffer;
    ff->max_vertices = max_vertices;
}

void ff_end(Fixed_Function *ff) {
    assert(ff->num_vertices < ff->max_vertices);
    // todo(josh): should this create() go in ff_begin?
    Buffer vertex_buffer = create_buffer(BT_VERTEX, ff->vertices, sizeof(ff->vertices[0]) * ff->num_vertices);
    u32 strides[1] = { sizeof(ff->vertices[0]) };
    u32 offsets[1] = { 0 };
    bind_vertex_buffers(&vertex_buffer, 1, 0, strides, offsets);
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
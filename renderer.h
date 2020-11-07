#pragma once

#include "math.h"
#include "basic.h"
#include "window.h"

void init_renderer(Window *window);

#define MAX_BOUND_TEXTURES 12

enum Texture_Format {
    TF_INVALID,
    TF_R8G8B8A8_UINT,
    TF_R8G8B8A8_UINT_SRGB,

    TF_COUNT,
};

enum Texture_Type {
    TT_INVALID,
    TT_2D,
    TT_3D,
    TT_CUBEMAP,

    TT_COUNT,
};

struct Texture_Description {
    int width;
    int height;
    bool render_target;
    Texture_Format format;
    Texture_Type type;
    // type
    // is_cpu_read_target
    byte *color_data;
};



#ifdef RENDER_BACKEND_DX11
#include <d3d11.h>
#include <d3dcompiler.h>

typedef ID3D11Buffer *Buffer;

typedef ID3D11VertexShader *Vertex_Shader;
typedef ID3D11PixelShader *Pixel_Shader;

typedef ID3D11Texture2D *Texture_Handle;

typedef ID3D11InputLayout *Vertex_Format;

struct DirectX {
    IDXGISwapChain *swap_chain_handle;
    ID3D11RenderTargetView *swap_chain_render_target_view;
    ID3D11Device *device;
    ID3D11DeviceContext *device_context;
    ID3D11RasterizerState *rasterizer;
    ID3D11DepthStencilState *no_depth_test_state;
    ID3D11SamplerState *linear_wrap_sampler;
    ID3D11SamplerState *linear_clamp_sampler;
    ID3D11SamplerState *point_wrap_sampler;
    ID3D11SamplerState *point_clamp_sampler;
    ID3D11BlendState *alpha_blend_state;
    ID3D11BlendState *no_alpha_blend_state;
    ID3D11ShaderResourceView *cur_srvs[MAX_BOUND_TEXTURES];
};
#endif



struct Texture {
    Texture_Handle handle;
    Texture_Description description;
#ifdef RENDER_BACKEND_DX11
    ID3D11ShaderResourceView *shader_resource_view;
#endif
};



enum Vertex_Field_Type {
    VFT_INVALID,
    VFT_INT,
    VFT_INT2,
    VFT_INT3,
    VFT_INT4,
    VFT_UINT,
    VFT_UINT2,
    VFT_UINT3,
    VFT_UINT4,
    VFT_FLOAT,
    VFT_FLOAT2,
    VFT_FLOAT3,
    VFT_FLOAT4,

    VFT_COUNT,
};

enum Vertex_Field_Step_Type {
    VFST_INVALID,
    VFST_PER_VERTEX,
    VFST_PER_INSTANCE,

    VFST_COUNT,
};

struct Vertex_Field {
    char *semantic;
    char *name;
    int offset;
    Vertex_Field_Type type;
    Vertex_Field_Step_Type step_type;
};



void          init_render_backend(Window *window);

Vertex_Format create_vertex_format(Vertex_Field *fields, int num_fields);
void          destroy_vertex_format(Vertex_Format format);
void          bind_vertex_format(Vertex_Format format);

Buffer        create_vertex_buffer(void *data, int data_len);
void          destroy_vertex_buffer(ID3D11Buffer *buffer);
void          bind_vertex_buffers(Buffer *buffers, int num_buffers, int stride);

Vertex_Shader compile_vertex_shader_from_file(wchar_t *filename);
Pixel_Shader  compile_pixel_shader_from_file(wchar_t *filename);
void          bind_shaders(Vertex_Shader vertex, Pixel_Shader pixel);

Texture       create_texture(Texture_Description desc);
void          bind_textures(Texture *textures, int slot, int num_textures);
void          unbind_all_textures();

void          prerender(int viewport_width, int viewport_height);
void          draw(int vertex_count, int start_vertex);
void          present(bool vsync);



struct FFVertex {
    Vector3 position;
    Vector3 tex_coord;
    Vector4 color;
};

struct Fixed_Function {
    FFVertex *vertices;
    int max_vertices;
    int num_vertices;
};

void ff_begin(Fixed_Function *ff, FFVertex *buffer, int max_vertices);
void ff_end(Fixed_Function *ff);
void ff_vertex(Fixed_Function *ff, Vector3 position);
void ff_tex_coord(Fixed_Function *ff, Vector3 tex_coord);
void ff_color(Fixed_Function *ff, Vector4 color);
void ff_next(Fixed_Function *ff);
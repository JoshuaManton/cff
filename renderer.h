#pragma once

#include "math.h"
#include "basic.h"
#include "window.h"

#define MAX_BOUND_TEXTURES 12

struct Vertex {
    Vector3 position;
    Vector3 tex_coord;
    Vector4 color;
};

enum Texture_Format {
    TF_R8G8B8A8_UINT,
    TF_R8G8B8A8_UINT_SRGB,

    TF_COUNT,
};

struct Texture_Description {
    int width;
    int height;
    bool render_target;
    Texture_Format format;
    // format
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
#endif

struct Texture {
    Texture_Handle handle;
    Texture_Description description;
#ifdef RENDER_BACKEND_DX11
    ID3D11ShaderResourceView *shader_resource_view;
#endif
};

#ifdef RENDER_BACKEND_DX11
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

    ID3D11InputLayout *vertex_format;
    ID3D11ShaderResourceView *cur_srvs[MAX_BOUND_TEXTURES];
};
#endif

Buffer        create_vertex_buffer(Vertex *vertices, int num_vertices);
void          destroy_vertex_buffer(ID3D11Buffer *buffer);
void          init_render_backend(Window *window);
Vertex_Shader compile_vertex_shader_from_file(wchar_t *filename);
Pixel_Shader  compile_pixel_shader_from_file(wchar_t *filename);
void          bind_shaders(Vertex_Shader vertex, Pixel_Shader pixel);
void          bind_vertex_buffers(Buffer *buffers, int num_buffers);
void          draw(int vertex_count, int start_vertex);
Texture       create_texture(Texture_Description desc);
void          bind_textures(Texture *textures, int slot, int num_textures);
void          unbind_all_textures();
void          prerender(int viewport_width, int viewport_height);
void          present(bool vsync);
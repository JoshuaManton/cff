#pragma once



#ifdef RENDER_BACKEND_DX11
#include <d3d11.h>
#include <d3dcompiler.h>

typedef ID3D11Buffer *Buffer;

typedef struct {
    ID3D11ShaderResourceView *shader_resource_view;
} Texture_Backend_Data;

typedef struct {
    ID3D11VertexShader *handle;
    ID3D10Blob *blob;
} Vertex_Shader;

typedef ID3D11PixelShader *Pixel_Shader;

typedef ID3D11Texture2D *Texture_Handle;

typedef ID3D11InputLayout *Vertex_Format;
#endif



#include "math.h"
#include "window.h"
#include "basic.h"

#define MAX_BOUND_TEXTURES 12
#define MAX_COLOR_BUFFERS   8

void init_renderer(Window *window);

enum Texture_Format {
    TF_INVALID,

    TF_R8_UINT,
    TF_R32_INT,
    TF_R32_FLOAT,
    TF_R16G16B16A16_FLOAT,
    TF_R32G32B32A32_FLOAT,
    TF_R8G8B8A8_UINT,
    TF_R8G8B8A8_UINT_SRGB,
    TF_DEPTH_STENCIL,

    TF_COUNT,
};

struct Texture_Format_Info {
    int pixel_size_in_bytes;
    int num_channels;
    bool is_depth_format;
};

enum Texture_Type {
    TT_INVALID,
    TT_2D,
    TT_3D,
    TT_CUBEMAP,

    TT_COUNT,
};

enum Texture_Wrap_Mode {
    TWM_INVALID,

    TWM_LINEAR_WRAP,
    TWM_LINEAR_CLAMP,
    TWM_POINT_WRAP,
    TWM_POINT_CLAMP,

    TWM_COUNT,
};

struct Texture_Description {
    int width;
    int height;
    bool render_target;
    Texture_Format format;
    Texture_Type type;
    // is_cpu_read_target
    Texture_Wrap_Mode wrap_mode;
    byte *color_data;
};

struct Texture {
    Texture_Handle handle;
    Texture_Description description;
    Texture_Backend_Data backend;
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

enum Buffer_Type {
    BT_INVALID,
    BT_VERTEX,
    BT_INDEX,
    BT_CONSTANT,
    BT_COUNT,
};



void init_render_backend(Window *window);
void create_color_and_depth_buffers(int width, int height, Texture_Format format, Texture_Wrap_Mode wrap_mode, Texture *out_color_buffer, Texture *out_depth_buffer);



Vertex_Format create_vertex_format(Vertex_Field *fields, int num_fields, Vertex_Shader shader);
void          destroy_vertex_format(Vertex_Format format);
void          bind_vertex_format(Vertex_Format format);

Buffer create_buffer(Buffer_Type type, void *data, int len);
void   update_buffer(Buffer buffer, void *data, int len);
void   destroy_buffer(Buffer buffer);
void   bind_vertex_buffers(Buffer *buffers, int num_buffers, u32 start_slot, u32 *strides, u32 *offsets);
void   bind_index_buffer(Buffer buffer, u32 slot);
void   bind_constant_buffers(Buffer *buffers, int num_buffers, u32 start_slot);

Vertex_Shader compile_vertex_shader_from_file(wchar_t *filename);
Pixel_Shader  compile_pixel_shader_from_file(wchar_t *filename);
void          bind_shaders(Vertex_Shader vertex, Pixel_Shader pixel);
// todo(josh): destroy shaders

Texture create_texture(Texture_Description desc);
void    destroy_texture(Texture texture);
void    bind_textures(Texture *textures, int num_textures, int slot);
void    unbind_all_textures();

void set_render_targets(Texture *color_buffers[MAX_COLOR_BUFFERS], Texture *depth_buffer);
void unset_render_targets();
void clear_bound_render_targets(Vector4 color);

void set_viewport(int x, int y, int width, int height);

void prerender();
void issue_draw_call(int vertex_count, int index_count, int instance_count = 0);
void present(bool vsync);

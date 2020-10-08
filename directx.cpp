#include <d3d11.h>
#include <d3dcompiler.h>

#define MAX_BOUND_TEXTURES 12

typedef ID3D11Buffer *Vertex_Buffer;
typedef ID3D11Buffer *Index_Buffer;
typedef ID3D11Buffer *Constant_Buffer;

typedef ID3D11VertexShader *Vertex_Shader;
typedef ID3D11PixelShader *Pixel_Shader;

typedef ID3D11Texture2D *Texture_Handle;

struct Texture {
    Texture_Handle handle;
    Texture_Description description;
    ID3D11ShaderResourceView *shader_resource_view;
};

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

DirectX directx;

ID3D11RenderTargetView *dx_create_render_target_view(ID3D11Texture2D *backing_texture) {
    D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc = {};
    render_target_view_desc.Format        = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // todo(josh): parameterize formats
    render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    ID3D11RenderTargetView *render_target_view = {};
    auto result = directx.device->CreateRenderTargetView(backing_texture, &render_target_view_desc, &render_target_view);
    assert(result == S_OK);
    return render_target_view;
}

ID3D11Buffer *create_vertex_buffer(Vertex *vertices, int num_vertices) {
    D3D11_BUFFER_DESC vertex_buffer_desc = {};
    vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    vertex_buffer_desc.ByteWidth = sizeof(Vertex) * num_vertices;
    vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertex_buffer_data = {};
    vertex_buffer_data.pSysMem = vertices;
    ID3D11Buffer *vertex_buffer = {};
    auto result = directx.device->CreateBuffer(&vertex_buffer_desc, &vertex_buffer_data, &vertex_buffer);
    assert(result == S_OK);
    return vertex_buffer;
}

void destroy_vertex_buffer(ID3D11Buffer *buffer) {
    buffer->Release();
}

void init_directx() {
    // Create swap chain
    DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
    swap_chain_desc.BufferCount                        = 2;
    swap_chain_desc.SwapEffect                         = DXGI_SWAP_EFFECT_FLIP_DISCARD; // todo(josh): use DXGI_SWAP_EFFECT_DISCARD (or something else) on non-Windows 10
    swap_chain_desc.BufferDesc.Width                   = (u32)window.width;
    swap_chain_desc.BufferDesc.Height                  = (u32)window.height;
    swap_chain_desc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.BufferDesc.RefreshRate.Numerator   = 60; // todo(josh): query monitor refresh rate. Zak: NOT the same as vsync
    swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
    swap_chain_desc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.OutputWindow                       = window.handle;
    swap_chain_desc.SampleDesc.Count                   = 1;
    swap_chain_desc.SampleDesc.Quality                 = 0;
    swap_chain_desc.Windowed                           = true;

    /*
    directx.swap_chain.width  = (int)swap_chain_desc.BufferDesc.Width;
    directx.swap_chain.height = (int)swap_chain_desc.BufferDesc.Height;
    directx.swap_chain.format = SWAP_CHAIN_FORMAT;
    */

    D3D_FEATURE_LEVEL requested_feature_level = D3D_FEATURE_LEVEL_11_0;
    D3D_FEATURE_LEVEL actual_feature_level = {};
    auto result = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_DEBUG,
        &requested_feature_level,
        1,
        D3D11_SDK_VERSION, // jake pls
        &swap_chain_desc,
        &directx.swap_chain_handle,
        &directx.device,
        &actual_feature_level,
        &directx.device_context);

    // todo(josh): if the hardware device fails, try making a WARP device
    assert(result == S_OK);

    ID3D11Texture2D *back_buffer_texture = {};
    result = directx.swap_chain_handle->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&back_buffer_texture);
    assert(result == S_OK);
    directx.swap_chain_render_target_view = dx_create_render_target_view(back_buffer_texture);
    back_buffer_texture->Release();

    // Make 2D rasterizer
    D3D11_RASTERIZER_DESC rasterizer_desc = {};
    rasterizer_desc.FillMode = D3D11_FILL_SOLID;
    rasterizer_desc.CullMode = D3D11_CULL_NONE;
    rasterizer_desc.DepthClipEnable = false;
    result = directx.device->CreateRasterizerState(&rasterizer_desc, &directx.rasterizer);
    assert(result == S_OK);

    // Make 2D depth stencil state
    D3D11_DEPTH_STENCIL_DESC depth_stencil_desc = {};
    depth_stencil_desc.DepthEnable                  = false;
    depth_stencil_desc.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK_ALL;
    depth_stencil_desc.DepthFunc                    = D3D11_COMPARISON_ALWAYS;
    depth_stencil_desc.StencilEnable                = true;
    depth_stencil_desc.StencilReadMask              = 0xff;
    depth_stencil_desc.StencilWriteMask             = 0xff;
    depth_stencil_desc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
    depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    depth_stencil_desc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
    depth_stencil_desc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
    depth_stencil_desc.BackFace.StencilFunc         = D3D11_COMPARISON_ALWAYS;
    depth_stencil_desc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
    depth_stencil_desc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_KEEP;
    depth_stencil_desc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
    result = directx.device->CreateDepthStencilState(&depth_stencil_desc, &directx.no_depth_test_state);
    assert(result == S_OK);

    // linear wrap sampler
    D3D11_SAMPLER_DESC linear_wrap_sampler_desc = {};
    linear_wrap_sampler_desc.Filter   = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    linear_wrap_sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    linear_wrap_sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    linear_wrap_sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    result = directx.device->CreateSamplerState(&linear_wrap_sampler_desc, &directx.linear_wrap_sampler);
    assert(result == S_OK);

    // linear clamp sampler
    D3D11_SAMPLER_DESC linear_clamp_sampler_desc = {};
    linear_clamp_sampler_desc.Filter   = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    linear_clamp_sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    linear_clamp_sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    linear_clamp_sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    result = directx.device->CreateSamplerState(&linear_clamp_sampler_desc, &directx.linear_clamp_sampler);
    assert(result == S_OK);

    // point wrap sampler
    D3D11_SAMPLER_DESC point_wrap_sampler_desc = {};
    point_wrap_sampler_desc.Filter   = D3D11_FILTER_MIN_MAG_MIP_POINT;
    point_wrap_sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    point_wrap_sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    point_wrap_sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    result = directx.device->CreateSamplerState(&point_wrap_sampler_desc, &directx.point_wrap_sampler);
    assert(result == S_OK);

    // point clamp sampler
    D3D11_SAMPLER_DESC point_clamp_sampler_desc = {};
    point_clamp_sampler_desc.Filter   = D3D11_FILTER_MIN_MAG_MIP_POINT;
    point_clamp_sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    point_clamp_sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    point_clamp_sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    result = directx.device->CreateSamplerState(&point_clamp_sampler_desc, &directx.point_clamp_sampler);
    assert(result == S_OK);

    // alpha blend state
    D3D11_BLEND_DESC alpha_blend_desc = {};
    // alpha_blend_desc.AlphaToCoverageEnable          = true;
    alpha_blend_desc.RenderTarget[0].BlendEnable    = true;
    alpha_blend_desc.RenderTarget[0].SrcBlend       = D3D11_BLEND_SRC_ALPHA;
    alpha_blend_desc.RenderTarget[0].DestBlend      = D3D11_BLEND_INV_SRC_ALPHA;
    alpha_blend_desc.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
    alpha_blend_desc.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_SRC_ALPHA;
    alpha_blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
    alpha_blend_desc.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;
    alpha_blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    result = directx.device->CreateBlendState(&alpha_blend_desc, &directx.alpha_blend_state);
    assert(result == S_OK);

    // no alpha blend state
    D3D11_BLEND_DESC no_blend_desc = {};
    no_blend_desc.RenderTarget[0].BlendEnable    = false;
    no_blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    result = directx.device->CreateBlendState(&no_blend_desc, &directx.no_alpha_blend_state);
    assert(result == S_OK);

    // create vertex format
    {
        char *dummy_vertex_shader =
        "struct VS_INPUT {\n"
        "  float3 position : SV_POSITION;\n"
        "  float3 texcoord : TEXCOORD;\n"
        "  float4 color    : COLOR;\n"
        "};"
        "struct PS_INPUT {\n"
        "  float4 position : SV_POSITION;\n"
        "  float3 texcoord : TEXCOORD;\n"
        "  float4 color    : COLOR;\n"
        "};"
        "PS_INPUT main(VS_INPUT input) {\n"
        "  PS_INPUT v;\n"
        "  v.position = float4(input.position, 1.0);\n"
        "  v.texcoord = input.texcoord;\n"
        "  v.color = input.color;\n"
        "  return v;\n"
        "};\n";
        ID3D10Blob *errors = {};
        ID3D10Blob *vertex_shader_blob = {};
        auto result = D3DCompile(dummy_vertex_shader, strlen(dummy_vertex_shader), nullptr, nullptr, nullptr, "main", "vs_4_0", 0, 0, &vertex_shader_blob, &errors);
        if (errors != nullptr) {
            auto str = (char *)errors->GetBufferPointer();
            printf(str);
            assert(false);
        }
        assert(result == S_OK);

        ID3D11VertexShader *vertex_shader = {};
        result = directx.device->CreateVertexShader(vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), nullptr, &vertex_shader);
        assert(result == S_OK);
        D3D11_INPUT_ELEMENT_DESC layout[] = {
            {"SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR",       0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
        };

        result = directx.device->CreateInputLayout(&layout[0], 3, vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), &directx.vertex_format);
        assert(result == S_OK);
        vertex_shader->Release();
        vertex_shader_blob->Release();
    }
}

Vertex_Shader compile_vertex_shader_from_file(wchar_t *filename) { // todo(josh): use a temp allocator to go from char * to wchar_t *
    ID3D10Blob *errors = {};
    ID3D10Blob *vertex_shader_blob = {};
    auto result = D3DCompileFromFile(filename, 0, 0, "main", "vs_5_0", 0, 0, &vertex_shader_blob, &errors);
    if (errors) {
        auto str = (char *)errors->GetBufferPointer();
        printf(str);
        assert(false);
    }
    assert(result == S_OK);
    ID3D11VertexShader *vertex_shader = {};
    result = directx.device->CreateVertexShader(vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), nullptr, &vertex_shader);
    assert(result == S_OK);
    if (errors) errors->Release();
    vertex_shader_blob->Release();
    return vertex_shader;
}

Pixel_Shader compile_pixel_shader_from_file(wchar_t *filename) { // todo(josh): use a temp allocator to go from char * to wchar_t *
    ID3D10Blob *errors = {};
    ID3D10Blob *pixel_shader_blob = {};
    auto result = D3DCompileFromFile(filename, 0, 0, "main", "ps_5_0", 0, 0, &pixel_shader_blob, &errors);
    if (errors) {
        auto str = (char *)errors->GetBufferPointer();
        printf(str);
        assert(false);
    }
    assert(result == S_OK);
    ID3D11PixelShader *pixel_shader = {};
    directx.device->CreatePixelShader(pixel_shader_blob->GetBufferPointer(), pixel_shader_blob->GetBufferSize(), nullptr, &pixel_shader);
    assert(result == S_OK);
    if (errors) errors->Release();
    pixel_shader_blob->Release();
    return pixel_shader;
}

void bind_shaders(Vertex_Shader vertex, Pixel_Shader pixel) {
    directx.device_context->VSSetShader(vertex, 0, 0);
    directx.device_context->PSSetShader(pixel, 0, 0);
}

void bind_vertex_buffers(Vertex_Buffer *buffers, int num_buffers) {
    u32 stride = sizeof(Vertex);
    u32 offset = 0;
    directx.device_context->IASetVertexBuffers(0, num_buffers, buffers, &stride, &offset);
}

void draw(int vertex_count, int start_vertex) {
    directx.device_context->Draw(vertex_count, start_vertex);
}

Texture create_texture(Texture_Description desc) {
    // todo(josh): check for max texture size?

    // Create texture
    D3D11_TEXTURE2D_DESC texture_desc = {};
    texture_desc.Width            = (u32)desc.width;
    texture_desc.Height           = (u32)desc.height;
    texture_desc.MipLevels        = 1;
    texture_desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
    texture_desc.SampleDesc.Count = 1;
    texture_desc.Usage = D3D11_USAGE_DEFAULT;

    texture_desc.ArraySize = 1; // texture2D only

    /*
    switch desc.type {
        case .Texture2D: {
        }
        case .Texture3D: {
            panic("Call dx_create_texture3d.");
        }
        case .Cubemap: {
            texture_desc.ArraySize = 6;
            texture_desc.MiscFlags |= d3d.D3D11_RESOURCE_MISC_TEXTURECUBE;
        }
        case .Invalid: fallthrough;
        case: panic(twrite(desc));
    }
    */

    texture_desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
    if (desc.render_target) {
        texture_desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
    }

    /*
    do_create_shader_resource := true;
    if texture_format_infos[desc.format].is_depth_format {
        do_create_shader_resource = false;
        texture_desc.BindFlags |= d3d.D3D11_BIND_DEPTH_STENCIL;
    }
    else {
        if !desc.is_cpu_read_target {
            texture_desc.BindFlags |= d3d.D3D11_BIND_SHADER_RESOURCE;
        }
        else {
            do_create_shader_resource = false;
        }
        if desc.render_target {
            texture_desc.BindFlags |= d3d.D3D11_BIND_RENDER_TARGET;
        }
    }
    */

    // Send the initial data
    ID3D11Texture2D *texture_handle = {};
    auto result = directx.device->CreateTexture2D(&texture_desc, nullptr, &texture_handle);
    assert(result == S_OK);
    if (desc.color_data) {
        // num_channels := cast(u32)texture_format_infos[desc.format].num_channels;
        directx.device_context->UpdateSubresource((ID3D11Resource *)texture_handle, 0, nullptr, desc.color_data, 4 * (u32)desc.width, 0); // todo(josh): replace the hardcoded for when we have different formats
    }

    // Create shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC texture_shader_resource_desc = {};
    texture_shader_resource_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texture_shader_resource_desc.Texture2D.MipLevels = 1;
    texture_shader_resource_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    /*
    switch desc.type {
        case .Texture2D: {
            texture_shader_resource_desc.ViewDimension = d3d.D3D11_SRV_DIMENSION_TEXTURE2D;
        }
        case .Texture3D: {
            panic("Call dx_create_texture3d.");
        }
        case .Cubemap: {
            texture_shader_resource_desc.ViewDimension = d3d.D3D11_SRV_DIMENSION_TEXTURECUBE;
        }
        case .Invalid: fallthrough;
        case: panic(twrite(desc));
    }
    */
    ID3D11ShaderResourceView *srv = {};
    result = directx.device->CreateShaderResourceView((ID3D11Resource *)texture_handle, &texture_shader_resource_desc, &srv);
    assert(result == S_OK);

    Texture texture = {};
    texture.description = desc;
    texture.handle = texture_handle;
    texture.shader_resource_view = srv;
    return texture;
}

void bind_textures(Texture *textures, int slot, int num_textures) {
    assert((slot + num_textures) < ARRAYSIZE(directx.cur_srvs));
    for (int i = slot; i < (slot + num_textures); i++) {
        if (directx.cur_srvs[i]) {
            directx.cur_srvs[i] = {};
        }

        Texture *texture = textures + i;
        if (texture) {
            assert(texture->shader_resource_view);
            directx.cur_srvs[i] = texture->shader_resource_view;
        }
    }
    directx.device_context->PSSetShaderResources((u32)slot, num_textures, &directx.cur_srvs[slot]);
}

void unbind_all_textures() {
    for (int i = 0; i < ARRAYSIZE(directx.cur_srvs); i++) {
        directx.cur_srvs[i] = {};
    }
    directx.device_context->PSSetShaderResources(0, ARRAYSIZE(directx.cur_srvs), &directx.cur_srvs[0]);
}

void prerender() {
    // if depth_buffer == nullptr {
    //     depth_texture_desc: Texture_Description;
    //     depth_texture_desc.type = .Texture2D;
    //     depth_texture_desc.format = .DEPTH_STENCIL;
    //     depth_texture_desc.width = main_window.width_int;
    //     depth_texture_desc.height = main_window.height_int;
    //     depth_texture := create_texture(depth_texture_desc); // todo(josh): this is probably leaking. need to store the depth texture in DirectX struct and release it when needed
    //     defer destroy_texture(depth_texture);
    //     directx.current_depth_stencil_view = _dx_create_depth_stencil_view(depth_texture.handle.texture_handle.(^ID3D11Texture2D), depth_texture_desc.format);
    // }
    // else {
    //     directx.current_depth_stencil_view = _dx_create_depth_stencil_view(depth_buffer.handle.texture_handle.(^ID3D11Texture2D), depth_buffer.format);
    // }

    directx.device_context->OMSetRenderTargets(1, &directx.swap_chain_render_target_view, nullptr);
    float r = 0.3 + ((float)sin(time_now()*1.0f) * (float)sin(time_now()*1.0f)) * 0.7;
    float g = 0.3 + ((float)sin(time_now()*0.7f) * (float)sin(time_now()*0.7f)) * 0.7;
    float b = 0.3 + ((float)sin(time_now()*0.6f) * (float)sin(time_now()*0.6f)) * 0.7;
    float color[4] = {r, g, b, 1.0f};
    directx.device_context->ClearRenderTargetView(directx.swap_chain_render_target_view, color);

    directx.device_context->IASetInputLayout(directx.vertex_format);

    directx.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = window.width;
    viewport.Height = window.height;
    directx.device_context->RSSetViewports(1, &viewport);

    directx.device_context->PSSetSamplers(0, 1, &directx.linear_wrap_sampler);
}

void postrender() {
    directx.swap_chain_handle->Present(1, 0);
}
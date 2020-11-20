struct DirectX {
    ID3D11Device *device;
    ID3D11DeviceContext *device_context;

    IDXGISwapChain *swap_chain_handle;
    int swap_chain_width;
    int swap_chain_height;

    Texture swap_chain_depth_buffer;

    ID3D11RasterizerState *no_cull_rasterizer;
    ID3D11RasterizerState *backface_cull_rasterizer;
    ID3D11DepthStencilState *depth_test_state;
    ID3D11DepthStencilState *no_depth_test_state;
    ID3D11SamplerState *linear_wrap_sampler;
    ID3D11SamplerState *linear_clamp_sampler;
    ID3D11SamplerState *point_wrap_sampler;
    ID3D11SamplerState *point_clamp_sampler;
    ID3D11BlendState *alpha_blend_state;
    ID3D11BlendState *no_alpha_blend_state;

    ID3D11RenderTargetView   *cur_rtvs[RB_MAX_COLOR_BUFFERS];
    Texture current_render_targets[RB_MAX_COLOR_BUFFERS]; // note(josh): for resolving MSAA
    ID3D11DepthStencilView   *cur_dsv;
    ID3D11ShaderResourceView *cur_srvs[RB_MAX_BOUND_TEXTURES];
    ID3D11UnorderedAccessView *cur_uavs[8]; // todo(josh): figure out a good constant
};

static DirectX directx;

static DXGI_FORMAT dx_texture_format_mapping[TF_COUNT];

ID3D11RenderTargetView *dx_create_render_target_view(ID3D11Texture2D *backing_texture, Texture_Format format, bool msaa) {
    ASSERT(format != TF_INVALID);
    D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc = {};
    render_target_view_desc.Format        = dx_texture_format_mapping[format];
    if (msaa) {
        render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
    }
    else {
        render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    }
    ID3D11RenderTargetView *render_target_view = {};
    auto result = directx.device->CreateRenderTargetView(backing_texture, &render_target_view_desc, &render_target_view);
    ASSERT(result == S_OK);
    return render_target_view;
}

ID3D11DepthStencilView *dx_create_depth_stencil_view(ID3D11Texture2D *backing_texture, Texture_Format format, bool msaa) {
    ASSERT(format != TF_INVALID);
    ASSERT(texture_format_infos[format].is_depth_format);
    D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc = {};
    depth_stencil_view_desc.Format = dx_texture_format_mapping[format];
    if (msaa) {
        depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
    }
    else {
        depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    }
    ID3D11DepthStencilView *depth_stencil_view = {};
    auto result = directx.device->CreateDepthStencilView((ID3D11Resource *)backing_texture, nullptr, &depth_stencil_view);
    ASSERT(result == S_OK);
    return depth_stencil_view;
}

#define SWAP_CHAIN_BUFFER_COUNT 2

void init_graphics_driver(Window *window) {
    dx_texture_format_mapping[TF_R8_UINT]            = DXGI_FORMAT_R8_UNORM;
    dx_texture_format_mapping[TF_R32_INT]            = DXGI_FORMAT_R32_SINT;
    dx_texture_format_mapping[TF_R32_FLOAT]          = DXGI_FORMAT_R32_FLOAT;
    dx_texture_format_mapping[TF_R16G16B16A16_FLOAT] = DXGI_FORMAT_R16G16B16A16_FLOAT;
    dx_texture_format_mapping[TF_R32G32B32A32_FLOAT] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    dx_texture_format_mapping[TF_R8G8B8A8_UINT]      = DXGI_FORMAT_R8G8B8A8_UNORM;
    dx_texture_format_mapping[TF_R8G8B8A8_UINT_SRGB] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    dx_texture_format_mapping[TF_DEPTH_STENCIL]      = DXGI_FORMAT_D24_UNORM_S8_UINT;

    // make sure all texture formats have a mapping
    for (int i = 0; i < ARRAYSIZE(dx_texture_format_mapping); i++) {
        if (dx_texture_format_mapping[i] == 0) {
            if ((Texture_Format)i != TF_INVALID && (Texture_Format)i != TF_COUNT) {
                printf("Missing dx texture format mapping for %d\n", i);
                ASSERT(false);
            }
        }
    }

    // Create swap chain
    DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
    swap_chain_desc.BufferCount                        = SWAP_CHAIN_BUFFER_COUNT;
    swap_chain_desc.SwapEffect                         = DXGI_SWAP_EFFECT_FLIP_DISCARD; // todo(josh): use DXGI_SWAP_EFFECT_DISCARD (or something else) on non-Windows 10
    swap_chain_desc.BufferDesc.Width                   = (u32)window->width;
    swap_chain_desc.BufferDesc.Height                  = (u32)window->height;
    swap_chain_desc.BufferDesc.Format                  = dx_texture_format_mapping[SWAP_CHAIN_FORMAT];
    swap_chain_desc.BufferDesc.RefreshRate.Numerator   = 60; // todo(josh): query monitor refresh rate.
    swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
    swap_chain_desc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.OutputWindow                       = window->handle;
    swap_chain_desc.SampleDesc.Count                   = 1;
    swap_chain_desc.SampleDesc.Quality                 = 0;
    swap_chain_desc.Windowed                           = true;

    directx.swap_chain_width  = window->width;
    directx.swap_chain_height = window->height;

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
    ASSERT(result == S_OK);

    Texture_Description depth_texture_desc = {};
    depth_texture_desc.type = TT_2D;
    depth_texture_desc.format = TF_DEPTH_STENCIL;
    depth_texture_desc.wrap_mode = TWM_POINT_CLAMP;
    depth_texture_desc.width = window->width;
    depth_texture_desc.height = window->height;
    depth_texture_desc.render_target = true;
    directx.swap_chain_depth_buffer = create_texture(depth_texture_desc);

    // Make no cull rasterizer
    D3D11_RASTERIZER_DESC no_cull_rasterizer_desc = {};
    no_cull_rasterizer_desc.FillMode = D3D11_FILL_SOLID;
    no_cull_rasterizer_desc.CullMode = D3D11_CULL_NONE;
    no_cull_rasterizer_desc.DepthClipEnable = false;
    no_cull_rasterizer_desc.MultisampleEnable = true; // todo(josh): can I just have multisample enabled on all rasterizers?
    result = directx.device->CreateRasterizerState(&no_cull_rasterizer_desc, &directx.no_cull_rasterizer);
    ASSERT(result == S_OK);

    // Make backface cull rasterizer
    D3D11_RASTERIZER_DESC backface_cull_rasterizer_desc = {};
    backface_cull_rasterizer_desc.FillMode = D3D11_FILL_SOLID;
    backface_cull_rasterizer_desc.CullMode = D3D11_CULL_BACK;
    backface_cull_rasterizer_desc.DepthClipEnable = true;
    backface_cull_rasterizer_desc.MultisampleEnable = true; // todo(josh): can I just have multisample enabled on all rasterizers?
    result = directx.device->CreateRasterizerState(&backface_cull_rasterizer_desc, &directx.backface_cull_rasterizer);
    ASSERT(result == S_OK);

    // Depth test state
    D3D11_DEPTH_STENCIL_DESC depth_test_stencil_desc = {};
    depth_test_stencil_desc.DepthEnable                  = true;
    depth_test_stencil_desc.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK_ALL;
    depth_test_stencil_desc.DepthFunc                    = D3D11_COMPARISON_LESS_EQUAL;
    depth_test_stencil_desc.StencilEnable                = true;
    depth_test_stencil_desc.StencilReadMask              = 0xff;
    depth_test_stencil_desc.StencilWriteMask             = 0xff;
    depth_test_stencil_desc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
    depth_test_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    depth_test_stencil_desc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
    depth_test_stencil_desc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
    depth_test_stencil_desc.BackFace.StencilFunc         = D3D11_COMPARISON_ALWAYS;
    depth_test_stencil_desc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
    depth_test_stencil_desc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_KEEP;
    depth_test_stencil_desc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
    result = directx.device->CreateDepthStencilState(&depth_test_stencil_desc, &directx.depth_test_state);
    ASSERT(result == S_OK);

    // No depth test state
    // todo(josh): should we disable stencil here?
    D3D11_DEPTH_STENCIL_DESC no_depth_test_stencil_desc = depth_test_stencil_desc;
    no_depth_test_stencil_desc.DepthEnable = false;
    no_depth_test_stencil_desc.DepthFunc   = D3D11_COMPARISON_ALWAYS;
    result = directx.device->CreateDepthStencilState(&no_depth_test_stencil_desc, &directx.no_depth_test_state);
    ASSERT(result == S_OK);

    // linear wrap sampler
    D3D11_SAMPLER_DESC linear_wrap_sampler_desc = {};
    linear_wrap_sampler_desc.Filter   = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    linear_wrap_sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    linear_wrap_sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    linear_wrap_sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    linear_wrap_sampler_desc.MinLOD = -FLT_MAX;
    linear_wrap_sampler_desc.MaxLOD = FLT_MAX;
    result = directx.device->CreateSamplerState(&linear_wrap_sampler_desc, &directx.linear_wrap_sampler);
    ASSERT(result == S_OK);

    // linear clamp sampler
    D3D11_SAMPLER_DESC linear_clamp_sampler_desc = {};
    linear_clamp_sampler_desc.Filter   = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    linear_clamp_sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    linear_clamp_sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    linear_clamp_sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    linear_clamp_sampler_desc.MinLOD = -FLT_MAX;
    linear_clamp_sampler_desc.MaxLOD = FLT_MAX;
    result = directx.device->CreateSamplerState(&linear_clamp_sampler_desc, &directx.linear_clamp_sampler);
    ASSERT(result == S_OK);

    // point wrap sampler
    D3D11_SAMPLER_DESC point_wrap_sampler_desc = {};
    point_wrap_sampler_desc.Filter   = D3D11_FILTER_MIN_MAG_MIP_POINT;
    point_wrap_sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    point_wrap_sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    point_wrap_sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    point_wrap_sampler_desc.MinLOD = -FLT_MAX;
    point_wrap_sampler_desc.MaxLOD = FLT_MAX;
    result = directx.device->CreateSamplerState(&point_wrap_sampler_desc, &directx.point_wrap_sampler);
    ASSERT(result == S_OK);

    // point clamp sampler
    D3D11_SAMPLER_DESC point_clamp_sampler_desc = {};
    point_clamp_sampler_desc.Filter   = D3D11_FILTER_MIN_MAG_MIP_POINT;
    point_clamp_sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    point_clamp_sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    point_clamp_sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    point_clamp_sampler_desc.MinLOD = -FLT_MAX;
    point_clamp_sampler_desc.MaxLOD = FLT_MAX;
    result = directx.device->CreateSamplerState(&point_clamp_sampler_desc, &directx.point_clamp_sampler);
    ASSERT(result == S_OK);

    // alpha blend state
    D3D11_BLEND_DESC alpha_blend_desc = {};
    // alpha_blend_desc.AlphaToCoverageEnable          = true;
    alpha_blend_desc.RenderTarget[0].BlendEnable    = true;
    alpha_blend_desc.RenderTarget[0].SrcBlend       = D3D11_BLEND_SRC_ALPHA;
    alpha_blend_desc.RenderTarget[0].DestBlend      = D3D11_BLEND_INV_SRC_ALPHA;
    alpha_blend_desc.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
    alpha_blend_desc.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_SRC_ALPHA;
    alpha_blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    alpha_blend_desc.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;
    alpha_blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    result = directx.device->CreateBlendState(&alpha_blend_desc, &directx.alpha_blend_state);
    ASSERT(result == S_OK);

    // no alpha blend state
    D3D11_BLEND_DESC no_blend_desc = {};
    no_blend_desc.RenderTarget[0].BlendEnable    = false;
    no_blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    result = directx.device->CreateBlendState(&no_blend_desc, &directx.no_alpha_blend_state);
    ASSERT(result == S_OK);
}

void set_viewport(int x, int y, int width, int height) {
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = width;
    viewport.Height = height;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1;
    directx.device_context->RSSetViewports(1, &viewport);
}

void set_depth_test(bool enabled) {
    if (enabled) {
        directx.device_context->OMSetDepthStencilState(directx.depth_test_state, 0);
    }
    else {
        directx.device_context->OMSetDepthStencilState(directx.no_depth_test_state, 0);
    }
}

void set_backface_cull(bool enabled) {
    if (enabled) {
        directx.device_context->RSSetState(directx.backface_cull_rasterizer);
    }
    else {
        directx.device_context->RSSetState(directx.no_cull_rasterizer);
    }
}

void set_primitive_topology(Primitive_Topology pt) {
    switch (pt) {
        case PT_TRIANGLE_LIST:  directx.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);  break;
        case PT_TRIANGLE_STRIP: directx.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP); break;
        case PT_LINE_LIST:      directx.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);      break;
        case PT_LINE_STRIP:     directx.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);     break;
        default: {
            ASSERTF(false, "Unknown Primitive_Topology: %d\n", pt);
        }
    }
}

void set_alpha_blend(bool enabled) {
    float blend_factor[4] = {1, 1, 1, 1};
    if (enabled) {
        directx.device_context->OMSetBlendState(directx.alpha_blend_state, blend_factor, 0xffffffff);
    }
    else {
        directx.device_context->OMSetBlendState(directx.no_alpha_blend_state, blend_factor, 0xffffffff);
    }
}

void ensure_swap_chain_size(int width, int height) {
    ASSERT(width != 0);
    ASSERT(height != 0);

    if (directx.swap_chain_width != width || directx.swap_chain_height != height) {
        printf("Resizing swap chain %dx%d...\n", width, height);

        auto result = directx.swap_chain_handle->ResizeBuffers(SWAP_CHAIN_BUFFER_COUNT, (u32)width, (u32)height, dx_texture_format_mapping[SWAP_CHAIN_FORMAT], 0);
        ASSERT(result == S_OK);
        directx.swap_chain_width  = width;
        directx.swap_chain_height = height;

        ensure_texture_size(&directx.swap_chain_depth_buffer, width, height);
    }
}

DXGI_FORMAT dx_vertex_field_type(Vertex_Field_Type vft) {
    switch (vft) {
        case VFT_INT:    return DXGI_FORMAT_R32_SINT;
        case VFT_INT2:   return DXGI_FORMAT_R32G32_SINT;
        case VFT_INT3:   return DXGI_FORMAT_R32G32B32_SINT;
        case VFT_INT4:   return DXGI_FORMAT_R32G32B32A32_SINT;
        case VFT_UINT:   return DXGI_FORMAT_R32_UINT;
        case VFT_UINT2:  return DXGI_FORMAT_R32G32_UINT;
        case VFT_UINT3:  return DXGI_FORMAT_R32G32B32_UINT;
        case VFT_UINT4:  return DXGI_FORMAT_R32G32B32A32_UINT;
        case VFT_FLOAT:  return DXGI_FORMAT_R32_FLOAT;
        case VFT_FLOAT2: return DXGI_FORMAT_R32G32_FLOAT;
        case VFT_FLOAT3: return DXGI_FORMAT_R32G32B32_FLOAT;
        case VFT_FLOAT4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
        default: {
            ASSERTF(false, "Unknown vertex format type: %d", vft);
            return DXGI_FORMAT_UNKNOWN;
        }
    }
}

Vertex_Format create_vertex_format(Vertex_Field *fields, int num_fields, Vertex_Shader shader) {
    ASSERTF(num_fields <= RB_MAX_VERTEX_FIELDS, "Too many vertex fields: %d vs %d. You can override the max vertex fields by defining RB_MAX_VERTEX_FIELDS before including render_backend.h", num_fields, RB_MAX_VERTEX_FIELDS);
    D3D11_INPUT_ELEMENT_DESC input_elements[RB_MAX_VERTEX_FIELDS] = {};
    for (int idx = 0; idx < num_fields; idx++) {
        D3D11_INPUT_ELEMENT_DESC *desc = &input_elements[idx];
        Vertex_Field *field = &fields[idx];

        D3D11_INPUT_CLASSIFICATION step_type = {};
        u32 step_rate = {};
        u32 buffer_slot = {};
        switch (field->step_type) {
            // todo(josh): parameterize step rate
            // todo(josh): parameterize buffer slot
            case VFST_PER_VERTEX:   buffer_slot = 0; step_rate = 0; step_type = D3D11_INPUT_PER_VERTEX_DATA;   break;
            case VFST_PER_INSTANCE: buffer_slot = 1; step_rate = 1; step_type = D3D11_INPUT_PER_INSTANCE_DATA; break;
            default: {
                ASSERTF(false, "Unknown step type: %d\n", field->step_type);
            }
        }

        desc->SemanticName = field->semantic;
        desc->Format = dx_vertex_field_type(field->type);
        desc->InputSlot = buffer_slot;
        desc->AlignedByteOffset = field->offset;
        desc->InputSlotClass = step_type;
        desc->InstanceDataStepRate = step_rate;
    }

    Vertex_Format vertex_format = {};
    auto result = directx.device->CreateInputLayout(input_elements, num_fields, shader.blob->GetBufferPointer(), shader.blob->GetBufferSize(), &vertex_format);
    ASSERTF(result == S_OK, "Failed to create input layout");
    return vertex_format;
}

void bind_vertex_format(Vertex_Format format) {
    directx.device_context->IASetInputLayout(format);
}

Buffer create_buffer(Buffer_Type type, void *data, int len) {
    D3D11_BUFFER_DESC buffer_desc = {};
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.ByteWidth = len;
    switch (type) {
        case BT_VERTEX:   { buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;   break; }
        case BT_INDEX:    { buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;    break; }
        case BT_CONSTANT: { buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; break; }
        default: {
            ASSERTF(false, "Unknown buffer type: %d", type);
        }
    }

    D3D11_SUBRESOURCE_DATA buffer_data = {};
    buffer_data.pSysMem = data;
    Buffer buffer = {};
    auto result = directx.device->CreateBuffer(&buffer_desc, data == nullptr ? nullptr : &buffer_data, &buffer);
    ASSERT(result == S_OK);
    return buffer;
}

void update_buffer(Buffer buffer, void *data, int len) {
    directx.device_context->UpdateSubresource((ID3D11Resource *)buffer, 0, nullptr, data, (u32)len, 0);
}

void bind_vertex_buffers(Buffer *buffers, int num_buffers, u32 start_slot, u32 *strides, u32 *offsets) {
    directx.device_context->IASetVertexBuffers(start_slot, num_buffers, buffers, strides, offsets);
}

void bind_index_buffer(Buffer buffer, u32 slot) {
    // todo(josh): parameterize index type
    directx.device_context->IASetIndexBuffer(buffer, DXGI_FORMAT_R32_UINT, slot);
}

void bind_constant_buffers(Buffer *buffers, int num_buffers, u32 start_slot) {
    directx.device_context->VSSetConstantBuffers(start_slot, num_buffers, buffers);
    directx.device_context->PSSetConstantBuffers(start_slot, num_buffers, buffers);
    directx.device_context->CSSetConstantBuffers(start_slot, num_buffers, buffers);
}

void destroy_buffer(Buffer buffer) {
    buffer->Release();
}

Vertex_Shader compile_vertex_shader_from_file(wchar_t *filename) { // todo(josh): use a temp allocator to go from char * to wchar_t *
    ID3D10Blob *errors = {};
    ID3D10Blob *vertex_shader_blob = {};
    auto result = D3DCompileFromFile(filename, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_WARNINGS_ARE_ERRORS, 0, &vertex_shader_blob, &errors);
    if (errors) {
        auto str = (char *)errors->GetBufferPointer();
        printf(str);
        ASSERT(false);
    }
    ASSERT(result == S_OK);
    ID3D11VertexShader *vertex_shader_handle = {};
    result = directx.device->CreateVertexShader(vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), nullptr, &vertex_shader_handle);
    ASSERT(result == S_OK);
    if (errors) errors->Release();
    Vertex_Shader vertex_shader = {};
    vertex_shader.handle = vertex_shader_handle;
    vertex_shader.blob = vertex_shader_blob;
    return vertex_shader;
}

Pixel_Shader compile_pixel_shader_from_file(wchar_t *filename) { // todo(josh): use a temp allocator to go from char * to wchar_t *
    ID3D10Blob *errors = {};
    ID3D10Blob *pixel_shader_blob = {};
    auto result = D3DCompileFromFile(filename, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_WARNINGS_ARE_ERRORS, 0, &pixel_shader_blob, &errors);
    if (errors) {
        auto str = (char *)errors->GetBufferPointer();
        printf(str);
        ASSERT(false);
    }
    ASSERT(result == S_OK);
    ID3D11PixelShader *pixel_shader = {};
    result = directx.device->CreatePixelShader(pixel_shader_blob->GetBufferPointer(), pixel_shader_blob->GetBufferSize(), nullptr, &pixel_shader);
    ASSERT(result == S_OK);
    if (errors) errors->Release();
    pixel_shader_blob->Release();
    return pixel_shader;
}

void bind_shaders(Vertex_Shader vertex, Pixel_Shader pixel) {
    directx.device_context->VSSetShader(vertex.handle, 0, 0);
    directx.device_context->PSSetShader(pixel, 0, 0);
}

Compute_Shader compile_compute_shader_from_file(wchar_t *filename) {
    ID3D10Blob *errors = {};
    ID3D10Blob *compute_blob = {};
    auto result = D3DCompileFromFile(filename, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "cs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_WARNINGS_ARE_ERRORS, 0, &compute_blob, &errors);
    if (errors) {
        auto str = (char *)errors->GetBufferPointer();
        printf(str);
        ASSERT(false);
    }
    ASSERT(result == S_OK);
    ID3D11ComputeShader *compute_shader = {};
    result = directx.device->CreateComputeShader(compute_blob->GetBufferPointer(), compute_blob->GetBufferSize(), nullptr, &compute_shader);
    ASSERT(result == S_OK);
    if (errors) errors->Release();
    compute_blob->Release();
    return compute_shader;
}

void bind_compute_shader(Compute_Shader shader) {
    directx.device_context->CSSetShader(shader, nullptr, 0);
}

ID3D11UnorderedAccessView *dx_create_unordered_access_view(Texture texture) {
    ASSERT(texture.description.uav);
    ID3D11UnorderedAccessView * uav = {};
    switch (texture.description.type) {
        case TT_2D: {
            D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
            uav_desc.Format = DXGI_FORMAT_UNKNOWN;
            uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
            auto result = directx.device->CreateUnorderedAccessView(texture.backend.handle_2d, &uav_desc, &uav);
            ASSERT(result == S_OK);
            break;
        }
        case TT_3D: {
            D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
            uav_desc.Format = DXGI_FORMAT_UNKNOWN;
            uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
            uav_desc.Texture3D.WSize = (u32)texture.description.depth;
            auto result = directx.device->CreateUnorderedAccessView(texture.backend.handle_3d, &uav_desc, &uav);
            ASSERT(result == S_OK);
            break;
        }
        case TT_CUBEMAP: {
            ASSERT(false && "unimplemented");
            // D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
            // uav_desc.Format = DXGI_FORMAT_UNKNOWN;
            // uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURECUBE;
            // uav_desc.Texture3D.WSize = (u32)texture.description.depth;
            // auto result = directx.device->CreateUnorderedAccessView(texture.backend.handle_3d, &uav_desc, &uav);
            // ASSERT(result == S_OK);
            break;
        }
        default: {
            ASSERT(false);
        }
    }
    ASSERT(uav != nullptr);
    return uav;
}

void bind_compute_uav(Texture texture, int slot) {
    ASSERT(slot < ARRAYSIZE(directx.cur_uavs));
    if (directx.cur_uavs[slot]) {
        directx.cur_uavs[slot]->Release();
        directx.cur_uavs[slot] = {};
    }

    if (texture.valid) {
        directx.cur_uavs[slot] = dx_create_unordered_access_view(texture);
    }

    directx.device_context->CSSetUnorderedAccessViews(slot, 1, &directx.cur_uavs[slot], nullptr);
}

void dispatch_compute(int x, int y, int z) {
    directx.device_context->Dispatch(x, y, z);
}

void destroy_vertex_shader(Vertex_Shader shader) {
    shader.handle->Release();
    shader.blob->Release();
}

void destroy_pixel_shader(Pixel_Shader shader) {
    shader->Release();
}

void destroy_compute_shader(Compute_Shader shader) {
    shader->Release();
}

void issue_draw_call(int vertex_count, int index_count, int instance_count) {
    if (instance_count == 0) {
        if (index_count > 0) {
            directx.device_context->DrawIndexed((u32)index_count, 0, 0);
        }
        else {
            directx.device_context->Draw((u32)vertex_count, 0);
        }
    }
    else {
        if (index_count > 0) {
            directx.device_context->DrawIndexedInstanced((u32)index_count, (u32)instance_count, 0, 0, 0);
        }
        else {
            directx.device_context->DrawInstanced((u32)vertex_count, (u32)instance_count, 0, 0);
        }
    }
}

Texture create_texture(Texture_Description desc) {
    // todo(josh): check for max texture size?
    ASSERT(desc.width > 0);
    ASSERT(desc.height > 0);

    if (desc.type == TT_INVALID) {
        desc.type = TT_2D;
    }

    if (desc.format == TF_INVALID) {
        desc.format = TF_R8G8B8A8_UINT;
    }

    if (desc.wrap_mode == TWM_INVALID) {
        desc.wrap_mode = TWM_POINT_CLAMP;
    }

    if (desc.sample_count == 0) {
        desc.sample_count = 1;
    }

    if (desc.mipmap_count == 0) {
        desc.mipmap_count = 1;
    }

    ASSERT(desc.mipmap_count <= 1 && "mipmaps are not supported yet");

    DXGI_FORMAT texture_format = dx_texture_format_mapping[desc.format];

    ID3D11Texture2D *texture_handle_2d = {};
    ID3D11Texture3D *texture_handle_3d = {};
    ID3D11Texture2D *msaa_handle_2d = {};
    ID3D11ShaderResourceView *shader_resource_view = {};
    ID3D11UnorderedAccessView *uav = {};
    switch (desc.type) {
        case TT_2D: {
            // Create texture
            D3D11_TEXTURE2D_DESC texture_desc = {};
            texture_desc.Width            = (u32)desc.width;
            texture_desc.Height           = (u32)desc.height;
            texture_desc.MipLevels        = desc.mipmap_count;
            texture_desc.Format           = texture_format;
            texture_desc.SampleDesc.Count = 1;
            texture_desc.Usage            = D3D11_USAGE_DEFAULT;
            texture_desc.ArraySize        = 1;

            if (texture_format_infos[desc.format].is_depth_format) {
                texture_desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
            }
            else {
                texture_desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
                if (desc.uav) {
                    texture_desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
                }
                if (desc.render_target) {
                    texture_desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
                }
            }

            u32 pixel_size = (u32)texture_format_infos[desc.format].pixel_size_in_bytes;
            D3D11_SUBRESOURCE_DATA subresource_data[6] = {
                {desc.color_data, pixel_size * (u32)desc.width,    0},
                {desc.color_data, pixel_size * (u32)desc.width/2,  0},
                {desc.color_data, pixel_size * (u32)desc.width/4,  0},
                {desc.color_data, pixel_size * (u32)desc.width/8,  0},
                {desc.color_data, pixel_size * (u32)desc.width/16, 0},
                {desc.color_data, pixel_size * (u32)desc.width/32, 0},
            };

            auto result = directx.device->CreateTexture2D(&texture_desc, desc.color_data == nullptr ? nullptr : &subresource_data[0], &texture_handle_2d);
            ASSERT(result == S_OK);

            if (desc.sample_count > 1) {
                ASSERT(desc.render_target);
                ASSERT(!desc.uav);
                D3D11_TEXTURE2D_DESC msaa_texture_desc = texture_desc;
                msaa_texture_desc.SampleDesc.Count = desc.sample_count;
                msaa_texture_desc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
                result = directx.device->CreateTexture2D(&msaa_texture_desc, nullptr, &msaa_handle_2d);
                ASSERT(result == S_OK);
            }

            break;
        }
        case TT_3D: {
            // Create texture
            D3D11_TEXTURE3D_DESC texture_desc = {};
            texture_desc.Width     = (u32)desc.width;
            texture_desc.Height    = (u32)desc.height;
            texture_desc.Depth     = (u32)desc.depth;
            texture_desc.MipLevels = desc.mipmap_count;
            texture_desc.Format    = texture_format;
            texture_desc.Usage     = D3D11_USAGE_DEFAULT;

            if (texture_format_infos[desc.format].is_depth_format) {
                texture_desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
            }
            else {
                texture_desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
                if (desc.uav) {
                    texture_desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
                }
                if (desc.render_target) {
                    texture_desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
                }
            }

            u32 pixel_size = (u32)texture_format_infos[desc.format].pixel_size_in_bytes;
            D3D11_SUBRESOURCE_DATA subresource_data[6] = {
                {desc.color_data, pixel_size * (u32)desc.width,    pixel_size * ((u32)desc.width)    * ((u32)desc.height)   },
                {desc.color_data, pixel_size * (u32)desc.width/2,  pixel_size * ((u32)desc.width/2)  * ((u32)desc.height/2) },
                {desc.color_data, pixel_size * (u32)desc.width/4,  pixel_size * ((u32)desc.width/4)  * ((u32)desc.height/4) },
                {desc.color_data, pixel_size * (u32)desc.width/8,  pixel_size * ((u32)desc.width/8)  * ((u32)desc.height/8) },
                {desc.color_data, pixel_size * (u32)desc.width/16, pixel_size * ((u32)desc.width/16) * ((u32)desc.height/16)},
                {desc.color_data, pixel_size * (u32)desc.width/32, pixel_size * ((u32)desc.width/32) * ((u32)desc.height/32)},
            };

            auto result = directx.device->CreateTexture3D(&texture_desc, desc.color_data == nullptr ? nullptr : &subresource_data[0], &texture_handle_3d);
            ASSERT(result == S_OK);

            break;
        }
        case TT_CUBEMAP: {
            // Create texture
            D3D11_TEXTURE2D_DESC texture_desc = {};
            texture_desc.Width            = (u32)desc.width;
            texture_desc.Height           = (u32)desc.height;
            texture_desc.MipLevels        = desc.mipmap_count;
            texture_desc.Format           = texture_format;
            texture_desc.SampleDesc.Count = 1;
            texture_desc.Usage            = D3D11_USAGE_DEFAULT;
            texture_desc.ArraySize        = 6;
            texture_desc.MiscFlags        |= D3D11_RESOURCE_MISC_TEXTURECUBE;

            if (texture_format_infos[desc.format].is_depth_format) {
                texture_desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
            }
            else {
                texture_desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
                if (desc.uav) {
                    texture_desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
                }
                if (desc.render_target) {
                    texture_desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
                }
            }

            u32 pixel_size = (u32)texture_format_infos[desc.format].pixel_size_in_bytes;
            D3D11_SUBRESOURCE_DATA subresource_data[6] = {
                {desc.color_data, pixel_size * (u32)desc.width,    0},
                {desc.color_data, pixel_size * (u32)desc.width/2,  0},
                {desc.color_data, pixel_size * (u32)desc.width/4,  0},
                {desc.color_data, pixel_size * (u32)desc.width/8,  0},
                {desc.color_data, pixel_size * (u32)desc.width/16, 0},
                {desc.color_data, pixel_size * (u32)desc.width/32, 0},
            };

            auto result = directx.device->CreateTexture2D(&texture_desc, desc.color_data == nullptr ? nullptr : &subresource_data[0], &texture_handle_2d);
            ASSERT(result == S_OK);

            if (desc.sample_count > 1) {
                ASSERT(desc.render_target);
                ASSERT(!desc.uav);
                D3D11_TEXTURE2D_DESC msaa_texture_desc = texture_desc;
                msaa_texture_desc.SampleDesc.Count = desc.sample_count;
                msaa_texture_desc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
                result = directx.device->CreateTexture2D(&msaa_texture_desc, nullptr, &msaa_handle_2d);
                ASSERT(result == S_OK);
            }

            break;
        }
    }

    Texture texture = {};
    texture.valid = true;
    texture.description = desc;
    texture.backend.handle_2d = texture_handle_2d;
    texture.backend.handle_msaa_2d = msaa_handle_2d;
    texture.backend.handle_3d = texture_handle_3d;
    texture.backend.shader_resource_view = shader_resource_view;
    texture.backend.uav = uav;
    return texture;
}

void destroy_texture(Texture texture) {
    if (texture.backend.handle_2d != nullptr) {
        texture.backend.handle_2d->Release();
    }
    if (texture.backend.handle_msaa_2d != nullptr) {
        texture.backend.handle_msaa_2d->Release();
    }
    if (texture.backend.handle_3d != nullptr) {
        texture.backend.handle_3d->Release();
    }
    if (texture.backend.shader_resource_view) {
        texture.backend.shader_resource_view->Release();
    }
    if (texture.backend.uav) {
        texture.backend.uav->Release();
    }
}

void set_cubemap_textures(Texture texture, byte *face_pixel_data[6]) {
    ASSERT(texture.description.type == TT_CUBEMAP);

    u32 num_channels = (u32)texture_format_infos[texture.description.format].num_channels;
    for (int idx = 0; idx < 6; idx++) {
        byte *face_data = face_pixel_data[idx];
        if (face_data != nullptr) {
            directx.device_context->UpdateSubresource((ID3D11Resource *)texture.backend.handle_2d, (u32)idx, nullptr, face_data, num_channels * (u32)texture.description.width, 0);
        }
    }
}

ID3D11ShaderResourceView *dx_create_shader_resource_view(Texture texture) {
    ASSERT(!texture_format_infos[texture.description.format].is_depth_format);
    ID3D11ShaderResourceView *shader_resource_view = {};
    switch (texture.description.type) {
        case TT_2D: {
            D3D11_SHADER_RESOURCE_VIEW_DESC texture_shader_resource_desc = {};
            texture_shader_resource_desc.Format = dx_texture_format_mapping[texture.description.format];
            texture_shader_resource_desc.Texture2D.MipLevels = texture.description.mipmap_count;
            texture_shader_resource_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            auto result = directx.device->CreateShaderResourceView((ID3D11Resource *)texture.backend.handle_2d, &texture_shader_resource_desc, &shader_resource_view);
            ASSERT(result == S_OK);
            break;
        }
        case TT_3D: {
            D3D11_SHADER_RESOURCE_VIEW_DESC texture_shader_resource_desc = {};
            texture_shader_resource_desc.Format = dx_texture_format_mapping[texture.description.format];
            texture_shader_resource_desc.Texture3D.MipLevels = texture.description.mipmap_count;
            texture_shader_resource_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
            auto result = directx.device->CreateShaderResourceView((ID3D11Resource *)texture.backend.handle_3d, &texture_shader_resource_desc, &shader_resource_view);
            ASSERT(result == S_OK);
            break;
        }
        case TT_CUBEMAP: {
            D3D11_SHADER_RESOURCE_VIEW_DESC texture_shader_resource_desc = {};
            texture_shader_resource_desc.Format = dx_texture_format_mapping[texture.description.format];
            texture_shader_resource_desc.TextureCube.MipLevels = texture.description.mipmap_count;
            texture_shader_resource_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
            auto result = directx.device->CreateShaderResourceView((ID3D11Resource *)texture.backend.handle_2d, &texture_shader_resource_desc, &shader_resource_view);
            ASSERT(result == S_OK);
            break;
        }
        default: {
            ASSERT(false);
        }
    }
    ASSERT(shader_resource_view != nullptr);
    return shader_resource_view;
}

void bind_texture(Texture texture, int slot) {
    ASSERT(slot < RB_MAX_BOUND_TEXTURES);
    if (directx.cur_srvs[slot]) {
        directx.cur_srvs[slot]->Release();
        directx.cur_srvs[slot] = {};
    }

    if (texture.valid) {
        directx.cur_srvs[slot] = dx_create_shader_resource_view(texture);
        switch (texture.description.wrap_mode) {
            case TWM_LINEAR_WRAP:  directx.device_context->PSSetSamplers(0, 1, &directx.linear_wrap_sampler);  break;
            case TWM_LINEAR_CLAMP: directx.device_context->PSSetSamplers(0, 1, &directx.linear_clamp_sampler); break;
            case TWM_POINT_WRAP:   directx.device_context->PSSetSamplers(0, 1, &directx.point_wrap_sampler);   break;
            case TWM_POINT_CLAMP:  directx.device_context->PSSetSamplers(0, 1, &directx.point_clamp_sampler);  break;
            default: {
                ASSERT(false && "No wrap mode specified for texture.");
            }
        }
    }
    directx.device_context->PSSetShaderResources(slot, 1, &directx.cur_srvs[slot]);
}

void unbind_all_textures() {
    for (int i = 0; i < ARRAYSIZE(directx.cur_srvs); i++) {
        if (directx.cur_srvs[i]) {
            directx.cur_srvs[i]->Release();
            directx.cur_srvs[i] = {};
        }
    }
    directx.device_context->PSSetShaderResources(0, ARRAYSIZE(directx.cur_srvs), &directx.cur_srvs[0]);
}

void copy_texture(Texture dst, Texture src) {
    ASSERT(false && "unimplemented");
    // if (src.description.sample_count > 1) {
    //     ASSERT(dst.description.format == src.description.format);
    //     directx.device_context->ResolveSubresource((ID3D11Resource *)dst.handle, 0, (ID3D11Resource *)src.handle, 0, dx_texture_format_mapping[dst.description.format]);
    // }
    // else {
    //     directx.device_context->CopyResource((ID3D11Resource *)dst.handle, (ID3D11Resource *)src.handle);
    // }
}

void set_render_targets(Texture *color_buffers, int num_color_buffers, Texture *depth_buffer) {
    ASSERT(num_color_buffers <= RB_MAX_COLOR_BUFFERS);

    unset_render_targets();

    int viewport_width  = 0;
    int viewport_height = 0;
    if (num_color_buffers > 0) {
        for (int i = 0; i < num_color_buffers; i++) {
            Texture color_buffer = color_buffers[i];
            ASSERT(directx.cur_rtvs[i] == nullptr);
            ASSERT(color_buffer.description.type == TT_2D);
            ASSERT(color_buffer.description.render_target);

            directx.current_render_targets[i] = color_buffer;
            if (viewport_width == 0) {
                viewport_width  = color_buffer.description.width;
                viewport_height = color_buffer.description.height;
            }
            bool msaa = color_buffer.description.sample_count > 1;
            if (msaa) {
                ASSERT(color_buffer.backend.handle_msaa_2d != nullptr);
            }
            directx.cur_rtvs[i] = dx_create_render_target_view(msaa ? color_buffer.backend.handle_msaa_2d : color_buffer.backend.handle_2d, color_buffer.description.format, msaa);
        }
    }
    else {
        viewport_width  = directx.swap_chain_width;
        viewport_height = directx.swap_chain_height;
        ID3D11Texture2D *back_buffer_texture = {};
        auto result = directx.swap_chain_handle->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&back_buffer_texture);
        ASSERT(result == S_OK);
        ASSERT(directx.cur_rtvs[0] == nullptr);
        directx.cur_rtvs[0] = dx_create_render_target_view(back_buffer_texture, SWAP_CHAIN_FORMAT, false); // todo(josh): swap chain msaa
        back_buffer_texture->Release();
    }

    Texture *depth_buffer_to_use = depth_buffer;
    if (depth_buffer_to_use == nullptr) {
        depth_buffer_to_use = &directx.swap_chain_depth_buffer;
    }
    ASSERT(directx.cur_dsv == nullptr);
    ASSERT(depth_buffer_to_use != nullptr);
    ASSERT(depth_buffer_to_use->description.type == TT_2D);
    ASSERT(depth_buffer_to_use->description.render_target);
    bool depth_msaa = depth_buffer_to_use->description.sample_count > 1;
    directx.cur_dsv = dx_create_depth_stencil_view(depth_msaa ? depth_buffer_to_use->backend.handle_msaa_2d : depth_buffer_to_use->backend.handle_2d, depth_buffer_to_use->description.format, depth_msaa);

    directx.device_context->OMSetRenderTargets(RB_MAX_COLOR_BUFFERS, &directx.cur_rtvs[0], directx.cur_dsv);

    ASSERT(viewport_width  != 0);
    ASSERT(viewport_height != 0);
    set_viewport(0, 0, viewport_width, viewport_height);
}

void unset_render_targets() {
    for (int i = 0; i < RB_MAX_COLOR_BUFFERS; i++) {
        ID3D11RenderTargetView *rtv = directx.cur_rtvs[i];
        if (directx.cur_rtvs[i] != nullptr) {
            Texture target = directx.current_render_targets[i];
            if (target.valid) {
                ASSERT(target.description.type == TT_2D);
                ASSERT(target.description.render_target);
                if (target.backend.handle_msaa_2d != nullptr) {
                    directx.device_context->ResolveSubresource((ID3D11Resource *)target.backend.handle_2d, 0, (ID3D11Resource *)target.backend.handle_msaa_2d, 0, dx_texture_format_mapping[target.description.format]);
                }
                directx.current_render_targets[i] = {};
            }

            directx.cur_rtvs[i]->Release();
            directx.cur_rtvs[i] = nullptr;
        }
    }
    if (directx.cur_dsv != nullptr) {
        directx.cur_dsv->Release();
        directx.cur_dsv = nullptr;
    }
}

void clear_bound_render_targets(Vector4 color) {
    for (int i = 0; i < RB_MAX_COLOR_BUFFERS; i++) {
        ID3D11RenderTargetView *rtv = directx.cur_rtvs[i];
        if (rtv != nullptr) {
            float color_elements[4] = { color.x, color.y, color.z, color.w };
            directx.device_context->ClearRenderTargetView(rtv, color_elements);
        }
    }
    if (directx.cur_dsv != nullptr) {
        directx.device_context->ClearDepthStencilView(directx.cur_dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
    }
}

void present(bool vsync) {
    directx.swap_chain_handle->Present(vsync, 0);
}


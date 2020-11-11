struct DirectX {
    ID3D11Device *device;
    ID3D11DeviceContext *device_context;

    IDXGISwapChain *swap_chain_handle;

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

    ID3D11RenderTargetView   *cur_rtvs[MAX_COLOR_BUFFERS];
    ID3D11DepthStencilView   *cur_dsv;
    ID3D11ShaderResourceView *cur_srvs[MAX_BOUND_TEXTURES];
};

static DirectX directx;

static DXGI_FORMAT dx_texture_format_mapping[TF_COUNT];

ID3D11RenderTargetView *dx_create_render_target_view(ID3D11Texture2D *backing_texture, Texture_Format format) {
    D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc = {};
    render_target_view_desc.Format        = dx_texture_format_mapping[format];
    render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    ID3D11RenderTargetView *render_target_view = {};
    auto result = directx.device->CreateRenderTargetView(backing_texture, &render_target_view_desc, &render_target_view);
    assert(result == S_OK);
    return render_target_view;
}

ID3D11DepthStencilView *dx_create_depth_stencil_view(ID3D11Texture2D *backing_texture, Texture_Format format) {
    assert(texture_format_infos[format].is_depth_format);
    D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc = {};
    depth_stencil_view_desc.Format = dx_texture_format_mapping[format];
    ID3D11DepthStencilView *depth_stencil_view = {};
    auto result = directx.device->CreateDepthStencilView((ID3D11Resource *)backing_texture, nullptr, &depth_stencil_view);
    assert(result == S_OK);
    return depth_stencil_view;
}

void init_graphics_driver(Window *window) {
    dx_texture_format_mapping[TF_R8_UINT]            = DXGI_FORMAT_R8_UNORM;
    dx_texture_format_mapping[TF_R8G8B8A8_UINT]      = DXGI_FORMAT_R8G8B8A8_UNORM;
    dx_texture_format_mapping[TF_R8G8B8A8_UINT_SRGB] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    dx_texture_format_mapping[TF_DEPTH_STENCIL]      = DXGI_FORMAT_D24_UNORM_S8_UINT;

    // make sure all texture formats have a mapping
    for (int i = 0; i < ARRAYSIZE(dx_texture_format_mapping); i++) {
        if (dx_texture_format_mapping[i] == 0) {
            if ((Texture_Format)i != TF_INVALID && (Texture_Format)i != TF_COUNT) {
                printf("Missing dx texture format mapping for %d\n", i);
                assert(false);
            }
        }
    }

    // Create swap chain
    DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
    swap_chain_desc.BufferCount                        = 2;
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

    Texture_Description depth_texture_desc = {};
    depth_texture_desc.type = TT_2D;
    depth_texture_desc.format = TF_DEPTH_STENCIL;
    depth_texture_desc.width = window->width;
    depth_texture_desc.height = window->height;
    directx.swap_chain_depth_buffer = create_texture(depth_texture_desc);

    // Make no cull rasterizer
    D3D11_RASTERIZER_DESC no_cull_rasterizer_desc = {};
    no_cull_rasterizer_desc.FillMode = D3D11_FILL_SOLID;
    no_cull_rasterizer_desc.CullMode = D3D11_CULL_NONE;
    no_cull_rasterizer_desc.DepthClipEnable = false;
    result = directx.device->CreateRasterizerState(&no_cull_rasterizer_desc, &directx.no_cull_rasterizer);
    assert(result == S_OK);

    // Make backface cull rasterizer
    D3D11_RASTERIZER_DESC backface_cull_rasterizer_desc = {};
    backface_cull_rasterizer_desc.FillMode = D3D11_FILL_SOLID;
    backface_cull_rasterizer_desc.CullMode = D3D11_CULL_BACK;
    backface_cull_rasterizer_desc.DepthClipEnable = true;
    result = directx.device->CreateRasterizerState(&backface_cull_rasterizer_desc, &directx.backface_cull_rasterizer);
    assert(result == S_OK);

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
    assert(result == S_OK);

    // No depth test state
    D3D11_DEPTH_STENCIL_DESC no_depth_test_stencil_desc = depth_test_stencil_desc;
    no_depth_test_stencil_desc.DepthEnable = false;
    no_depth_test_stencil_desc.DepthFunc   = D3D11_COMPARISON_ALWAYS;
    result = directx.device->CreateDepthStencilState(&no_depth_test_stencil_desc, &directx.no_depth_test_state);
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
            assert(false && "unknown format");
            return (DXGI_FORMAT)0;
        }
    }
}

Vertex_Format create_vertex_format(Vertex_Field *fields, int num_fields, Vertex_Shader shader) {
    // todo(josh): this doesn't have to be an allocation. we could just have a MAX_VERTEX_FIELDS?
    D3D11_INPUT_ELEMENT_DESC *input_elements = MAKE(default_allocator(), D3D11_INPUT_ELEMENT_DESC, num_fields);
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
                printf("%d\n", field->step_type);
                assert(false && "unknown step type");
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
    assert(result == S_OK);
    free(default_allocator(), input_elements);
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
            assert(false && "unknown buffer type");
        }
    }

    D3D11_SUBRESOURCE_DATA buffer_data = {};
    buffer_data.pSysMem = data;
    Buffer buffer = {};
    auto result = directx.device->CreateBuffer(&buffer_desc, data == nullptr ? nullptr : &buffer_data, &buffer);
    assert(result == S_OK);
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
}

void destroy_buffer(Buffer buffer) {
    buffer->Release();
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
    ID3D11VertexShader *vertex_shader_handle = {};
    result = directx.device->CreateVertexShader(vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), nullptr, &vertex_shader_handle);
    assert(result == S_OK);
    if (errors) errors->Release();
    Vertex_Shader vertex_shader = {};
    vertex_shader.handle = vertex_shader_handle;
    vertex_shader.blob = vertex_shader_blob;
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
    directx.device_context->VSSetShader(vertex.handle, 0, 0);
    directx.device_context->PSSetShader(pixel, 0, 0);
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

    assert(desc.type == TT_2D && "only 2D textures supported right now");
    assert(desc.format != TF_INVALID);

    DXGI_FORMAT texture_format = dx_texture_format_mapping[desc.format];

    // Create texture
    D3D11_TEXTURE2D_DESC texture_desc = {};
    texture_desc.Width            = (u32)desc.width;
    texture_desc.Height           = (u32)desc.height;
    texture_desc.MipLevels        = 1;
    texture_desc.Format           = texture_format;
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

    bool do_create_shader_resource = true;
    if (texture_format_infos[desc.format].is_depth_format) {
        do_create_shader_resource = false;
        texture_desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
    }
    else {
        texture_desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
        if (desc.render_target) {
            texture_desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
        }
    }

    /*
    do_create_shader_resource := true;
    if texture_format_infos[desc.format].is_depth_format {
        do_create_shader_resource = false;
        texture_desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
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
        directx.device_context->UpdateSubresource((ID3D11Resource *)texture_handle, 0, nullptr, desc.color_data, texture_format_infos[desc.format].num_channels * (u32)desc.width, 0); // todo(josh): replace the hardcoded for when we have different formats
    }

    // Create shader resource view
    ID3D11ShaderResourceView *shader_resource_view = {};
    if (do_create_shader_resource) {
        D3D11_SHADER_RESOURCE_VIEW_DESC texture_shader_resource_desc = {};
        texture_shader_resource_desc.Format = texture_format;
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
        result = directx.device->CreateShaderResourceView((ID3D11Resource *)texture_handle, &texture_shader_resource_desc, &shader_resource_view);
        assert(result == S_OK);
    }

    Texture texture = {};
    texture.description = desc;
    texture.handle = texture_handle;
    texture.shader_resource_view = shader_resource_view;
    return texture;
}

void destroy_texture(Texture texture) {
    texture.handle->Release();
    if (texture.shader_resource_view) {
        texture.shader_resource_view->Release();
    }
}

void bind_textures(Texture *textures, int num_textures, int start_slot) {
    assert((start_slot + num_textures) < MAX_BOUND_TEXTURES);
    for (int i = 0; i < num_textures; i++) {
        int slot = start_slot + i;
        if (directx.cur_srvs[slot]) {
            directx.cur_srvs[slot] = {};
        }

        Texture *texture = textures + i;
        if (texture) {
            assert(texture->shader_resource_view);
            directx.cur_srvs[slot] = texture->shader_resource_view;
        }
    }
    directx.device_context->PSSetShaderResources((u32)start_slot, num_textures, &directx.cur_srvs[start_slot]);
}

void unbind_all_textures() {
    for (int i = 0; i < ARRAYSIZE(directx.cur_srvs); i++) {
        directx.cur_srvs[i] = {};
    }
    directx.device_context->PSSetShaderResources(0, ARRAYSIZE(directx.cur_srvs), &directx.cur_srvs[0]);
}

void set_render_targets(Texture *color_buffers[MAX_COLOR_BUFFERS], Texture *depth_buffer) {
    unset_render_targets();

    if (color_buffers != nullptr) {
        for (int i = 0; i < MAX_COLOR_BUFFERS; i++) {
            Texture *color_buffer = color_buffers[i];
            assert(directx.cur_rtvs[i] == nullptr);
            if (color_buffer != nullptr) {
                assert(color_buffer->description.render_target);
                directx.cur_rtvs[i] = dx_create_render_target_view(color_buffer->handle, color_buffer->description.format);
            }
        }
    }
    else {
        ID3D11Texture2D *back_buffer_texture = {};
        auto result = directx.swap_chain_handle->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&back_buffer_texture);
        assert(result == S_OK);
        assert(directx.cur_rtvs[0] == nullptr);
        directx.cur_rtvs[0] = dx_create_render_target_view(back_buffer_texture, SWAP_CHAIN_FORMAT); // todo(josh): srgb backbuffer
        back_buffer_texture->Release();
    }

    Texture *depth_buffer_to_use = depth_buffer;
    if (depth_buffer_to_use == nullptr) {
        depth_buffer_to_use = &directx.swap_chain_depth_buffer;
    }
    assert(directx.cur_dsv == nullptr);
    directx.cur_dsv = dx_create_depth_stencil_view(depth_buffer_to_use->handle, depth_buffer_to_use->description.format);

    directx.device_context->OMSetRenderTargets(MAX_COLOR_BUFFERS, &directx.cur_rtvs[0], directx.cur_dsv);
}

void unset_render_targets() {
    for (int i = 0; i < MAX_COLOR_BUFFERS; i++) {
        ID3D11RenderTargetView *rtv = directx.cur_rtvs[i];
        if (directx.cur_rtvs[i] != nullptr) {
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
    for (int i = 0; i < MAX_COLOR_BUFFERS; i++) {
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

void prerender(int viewport_width, int viewport_height) {
    directx.device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = viewport_width;
    viewport.Height = viewport_height;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1;
    directx.device_context->RSSetViewports(1, &viewport);
    directx.device_context->PSSetSamplers(0, 1, &directx.linear_wrap_sampler);
    directx.device_context->RSSetState(directx.backface_cull_rasterizer);
    directx.device_context->OMSetDepthStencilState(directx.depth_test_state, 0);

    float blend_factor[4] = {0, 0, 0, 0};
    u32 sample_mask = 0xffffffff;
    directx.device_context->OMSetBlendState(directx.alpha_blend_state, blend_factor, sample_mask);
}

void present(bool vsync) {
    directx.swap_chain_handle->Present(vsync, 0);
}


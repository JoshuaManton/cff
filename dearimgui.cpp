#ifdef DEVELOPER

#include "external/dearimgui/imgui.h"

struct ImguiDX {
    ID3D11Device                *Device;
    ID3D11DeviceContext         *DeviceContext;
    // IDXGIFactory             *Factory;
    ID3D11Buffer                *VB;
    ID3D11Buffer                *IB;
    ID3D11VertexShader          *VertexShader;
    ID3D11InputLayout           *InputLayout;
    ID3D11Buffer                *VertexConstantBuffer;
    ID3D11PixelShader           *PixelShader;
    ID3D11SamplerState          *FontSampler;
    // ID3D11ShaderResourceView *FontTextureView;
    Texture                     FontTexture;
    ID3D11RasterizerState       *RasterizerState;
    ID3D11BlendState            *BlendState;
    ID3D11DepthStencilState     *DepthStencilState;
    i32                         VertexBufferSize;
    i32                         IndexBufferSize;
};

struct Imgui_CBuffer {
    Matrix4 mvp;
};

ImguiDX imgui_dx;

bool init_dear_imgui() {
    ImGui::CreateContext();

    imgui_dx.Device        = directx.device;
    imgui_dx.DeviceContext = directx.device_context;
    imgui_dx.Device       ->AddRef();
    imgui_dx.DeviceContext->AddRef();

    auto io = ImGui::GetIO();

    io.KeyMap[ImGuiKey_Tab]        = (i32)INPUT_TAB;
    io.KeyMap[ImGuiKey_LeftArrow]  = (i32)INPUT_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = (i32)INPUT_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow]    = (i32)INPUT_UP;
    io.KeyMap[ImGuiKey_DownArrow]  = (i32)INPUT_DOWN;
    io.KeyMap[ImGuiKey_PageUp]     = (i32)INPUT_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown]   = (i32)INPUT_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home]       = (i32)INPUT_HOME;
    io.KeyMap[ImGuiKey_End]        = (i32)INPUT_END;
    io.KeyMap[ImGuiKey_Delete]     = (i32)INPUT_DELETE;
    io.KeyMap[ImGuiKey_Backspace]  = (i32)INPUT_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter]      = (i32)INPUT_ENTER;
    io.KeyMap[ImGuiKey_Escape]     = (i32)INPUT_ESCAPE;
    io.KeyMap[ImGuiKey_A]          = (i32)INPUT_A;
    io.KeyMap[ImGuiKey_C]          = (i32)INPUT_C;
    io.KeyMap[ImGuiKey_V]          = (i32)INPUT_V;
    io.KeyMap[ImGuiKey_X]          = (i32)INPUT_X;
    io.KeyMap[ImGuiKey_Y]          = (i32)INPUT_Y;
    io.KeyMap[ImGuiKey_Z]          = (i32)INPUT_Z;

/*
    // Get factory from device
    pDXGIDevice:  ^IDXGIDevice;
    pDXGIAdapter: ^IDXGIAdapter;
    pFactory:     ^IDXGIFactory;

    if device.QueryInterface(device, IID_PPV_ARGS(&pDXGIDevice)) == S_OK)
        if (pDXGIDevice->GetParent(IID_PPV_ARGS(&pDXGIAdapter)) == S_OK)
            if (pDXGIAdapter->GetParent(IID_PPV_ARGS(&pFactory)) == S_OK)
            {
                g_pd3dDevice = device;
                g_pd3dDeviceContext = device_context;
                g_pFactory = pFactory;
            }



    if (pDXGIDevice) pDXGIDevice->Release();
    if (pDXGIAdapter) pDXGIAdapter->Release();

*/

    // init dx
    {
        assert(imgui_dx.Device        != nullptr);
        assert(imgui_dx.DeviceContext != nullptr);
        assert(imgui_dx.FontSampler   == nullptr);

        // Create the vertex shader
        {
            const char *vertex_shader = ""
                "cbuffer vertexBuffer : register(b0)\n"
                "{\n"
                "  float4x4 ProjectionMatrix;\n"
                "};\n"
                "struct VS_INPUT\n"
                "{\n"
                "  float2 pos : POSITION;\n"
                "  float4 col : COLOR0;\n"
                "  float2 uv  : TEXCOORD0;\n"
                "};\n"
                "struct PS_INPUT\n"
                "{\n"
                "  float4 pos : SV_POSITION;\n"
                "  float4 col : COLOR0;\n"
                "  float2 uv  : TEXCOORD0;\n"
                "};\n"
                "PS_INPUT main(VS_INPUT input)\n"
                "{\n"
                "  PS_INPUT output;\n"
                "  output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));\n"
                "  output.col = input.col;\n"
                "  output.uv  = input.uv;\n"
                "  return output;\n"
                "}\n";

            ID3D10Blob *vertex_shader_blob = {};
            ID3D10Blob *errors = {};
            auto result = D3DCompile(vertex_shader, strlen(vertex_shader), nullptr, nullptr, nullptr, "main", "vs_4_0", 0, 0, &vertex_shader_blob, &errors);
            if (errors) {
                auto str = (char *)errors->GetBufferPointer();
                printf(str);
                // ASSERT(false);
            }
            assert(result == S_OK);
            defer(vertex_shader_blob->Release());

            result = imgui_dx.Device->CreateVertexShader(vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), nullptr, &imgui_dx.VertexShader);
            assert(result == S_OK);

            // Create the input layout
            D3D11_INPUT_ELEMENT_DESC local_layout[3] = {
                {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, 8,  D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
            };
            result = imgui_dx.Device->CreateInputLayout(&local_layout[0], 3, vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), &imgui_dx.InputLayout);
            assert(result == S_OK);

            // Create the constant buffer
            {
                D3D11_BUFFER_DESC desc = {};
                desc.ByteWidth      = sizeof(Imgui_CBuffer);
                desc.Usage          = D3D11_USAGE_DYNAMIC;
                desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
                desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                imgui_dx.Device->CreateBuffer(&desc, nullptr, &imgui_dx.VertexConstantBuffer);
            }
        }

        // Create the pixel shader
        {
            const char *pixel_shader_text = ""
                "struct PS_INPUT {\n"
                "    float4 pos : SV_POSITION;\n"
                "    float4 col : COLOR0;\n"
                "    float2 uv  : TEXCOORD0;\n"
                "};\n"
                "sampler sampler0;\n"
                "Texture2D texture0;\n"
                "float4 main(PS_INPUT input) : SV_Target {\n"
                "    float4 out_col = input.col * texture0.Sample(sampler0, input.uv);\n"
                "    float a = out_col.a;\n"
                // "    return float4(pow(abs(out_col.xyz), 2.2), a);\n"
                "    return out_col;\n"
                "}\n";

            ID3D10Blob *pixel_shader_blob = {};
            ID3D10Blob *errors = {};
            auto result = D3DCompile(pixel_shader_text, strlen(pixel_shader_text), nullptr, nullptr, nullptr, "main", "ps_4_0", 0, 0, &pixel_shader_blob, &errors);
            if (errors) {
                auto str = (char *)errors->GetBufferPointer();
                printf(str);
                // ASSERT(false);
            }
            assert(result == S_OK);
            defer(pixel_shader_blob->Release());

            result = imgui_dx.Device->CreatePixelShader(pixel_shader_blob->GetBufferPointer(), pixel_shader_blob->GetBufferSize(), nullptr, &imgui_dx.PixelShader);
            assert(result == S_OK);
        }

        // Create the blending setup
        {
            D3D11_BLEND_DESC desc = {};
            desc.AlphaToCoverageEnable                 = false;
            desc.RenderTarget[0].BlendEnable           = true;
            desc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
            desc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
            desc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
            desc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_INV_SRC_ALPHA;
            desc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
            desc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
            desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            auto result = imgui_dx.Device->CreateBlendState(&desc, &imgui_dx.BlendState);
            assert(result == S_OK);
        }

        // Create the rasterizer state
        {
            D3D11_RASTERIZER_DESC desc = {};
            desc.FillMode = D3D11_FILL_SOLID;
            desc.CullMode = D3D11_CULL_NONE;
            desc.ScissorEnable = true;
            desc.DepthClipEnable = true;
            auto result = imgui_dx.Device->CreateRasterizerState(&desc, &imgui_dx.RasterizerState);
            assert(result == S_OK);
        }

        // Create depth-stencil State
        {
            D3D11_DEPTH_STENCIL_DESC desc = {};
            desc.DepthEnable                  = false;
            desc.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK_ALL;
            desc.DepthFunc                    = D3D11_COMPARISON_ALWAYS;
            desc.StencilEnable                = false;
            desc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
            desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
            desc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
            desc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
            desc.BackFace                     = desc.FrontFace;
            auto result = imgui_dx.Device->CreateDepthStencilState(&desc, &imgui_dx.DepthStencilState);
            assert(result == S_OK);
        }
    }

    // init fonts
    {
        // Upload texture to graphics system
        {
            byte *pixels = {};
            i32 width = 0;
            i32 height = 0;
            io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

            Texture_Description desc = {};
            desc.width = width;
            desc.height = height;
            desc.format = TF_R8G8B8A8_UINT;
            desc.color_data = pixels;
            imgui_dx.FontTexture = create_texture(desc);
            // todo(josh): we probably @Leak this ^^^ but who cares?

            /*
            desc: D3D11_TEXTURE2D_DESC;
            desc.Width            = cast(u32)width;
            desc.Height           = cast(u32)height;
            desc.MipLevels        = 1;
            desc.ArraySize        = 1;
            desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.SampleDesc.Count = 1;
            desc.Usage            = D3D11_USAGE_DEFAULT;
            desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags   = 0;

            pTexture: ^ID3D11Texture2D;
            subResource: D3D11_SUBRESOURCE_DATA;
            subResource.pSysMem          = pixels;
            subResource.SysMemPitch      = desc.Width * 4;
            subResource.SysMemSlicePitch = 0;
            result := imgui_dx.Device.CreateTexture2D(imgui_dx.Device, &desc, &subResource, &pTexture);
            assert(result == S_OK);

            // Create texture view
            srvDesc: D3D11_SHADER_RESOURCE_VIEW_DESC;
            srvDesc.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
            srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels       = desc.MipLevels;
            srvDesc.Texture2D.MostDetailedMip = 0;
            result = imgui_dx.Device.CreateShaderResourceView(imgui_dx.Device, cast(^ID3D11Resource)pTexture, &srvDesc, &imgui_dx.FontTextureView);
            assert(result == S_OK);
            pTexture.Release(pTexture);
            */
        }

        // Store our identifier
        io.Fonts->TexID = (ImTextureID)&imgui_dx.FontTexture;

        // Create texture sampler
        {
            D3D11_SAMPLER_DESC desc = {};
            desc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            desc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
            desc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
            desc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
            desc.MipLODBias     = 0;
            desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
            desc.MinLOD         = 0;
            desc.MaxLOD         = 0;
            auto result = imgui_dx.Device->CreateSamplerState(&desc, &imgui_dx.FontSampler);
            assert(result == S_OK);
        }
    }

    ImGui::StyleColorsDark();
    return true;
}

void dear_imgui_new_frame(Window *window, float dt) {
    assert(imgui_dx.FontSampler != nullptr);

    auto& io = ImGui::GetIO();
    io.DisplaySize.x = window->width;
    io.DisplaySize.y = window->height;

    if (window->is_focused) {
        io.MousePos = ImVec2(window->mouse_position_pixel.x, window->height - window->mouse_position_pixel.y);
        io.MouseDown[0] = get_input(window, INPUT_MOUSE_LEFT);
        io.MouseDown[1] = get_input(window, INPUT_MOUSE_RIGHT);
        io.MouseWheel   = window->mouse_scroll;
        if (io.WantCaptureMouse) {
            window->mouse_scroll = 0;
        }

        io.KeyCtrl  = get_input(window, INPUT_CONTROL);
        io.KeyShift = get_input(window, INPUT_SHIFT);
        io.KeyAlt   = get_input(window, INPUT_ALT);
        io.KeySuper = get_input(window, INPUT_LEFT_WINDOWS) || get_input(window, INPUT_RIGHT_WINDOWS);

        // todo(josh): do we care about this?
        // for input, idx in Input {
        //     io.keys_down[idx] = get_global_input(input);
        // }
    } else {
        io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

        io.MouseDown[0] = false;
        io.MouseDown[1] = false;
        io.MouseWheel   = 0;
        io.KeyCtrl  = false;
        io.KeyShift = false;
        io.KeyAlt   = false;
        io.KeySuper = false;

        // for i in 0..511 {
        //     io.keys_down[i] = false;
        // }
    }

    // ctx.imgui_state.mouse_wheel_delta = 0;
    io.DeltaTime = dt;
    ImGui::NewFrame();
}

// Backup DX state that will be modified to restore it afterwards (unfortunately this is very ugly looking and verbose. Close your eyes!)
struct BACKUP_DX11_STATE {
    u32                                                                      ScissorRectsCount, ViewportsCount;
    D3D11_RECT                                                               ScissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    D3D11_VIEWPORT                                                           Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    ID3D11RasterizerState                                                    *RS;
    ID3D11BlendState                                                         *BlendState;
    FLOAT                                                                    BlendFactor[4];
    u32                                                                      SampleMask;
    u32                                                                      StencilRef;
    ID3D11DepthStencilState                                                  *DepthStencilState;
    ID3D11ShaderResourceView                                                 *PSShaderResource;
    ID3D11SamplerState                                                       *PSSampler;
    ID3D11PixelShader                                                        *PS;
    ID3D11VertexShader                                                       *VS;
    ID3D11GeometryShader                                                     *GS;
    u32                                                                      PSInstancesCount, VSInstancesCount, GSInstancesCount;
    ID3D11ClassInstance                                                      *PSInstances[256];
    ID3D11ClassInstance                                                      *VSInstances[256];
    ID3D11ClassInstance                                                      *GSInstances[256];
    D3D11_PRIMITIVE_TOPOLOGY                                                 PrimitiveTopology;
    ID3D11Buffer                                                             *IndexBuffer;
    ID3D11Buffer                                                             *VertexBuffer;
    ID3D11Buffer                                                             *VSConstantBuffer;
    u32                                                                      IndexBufferOffset, VertexBufferStride, VertexBufferOffset;
    DXGI_FORMAT                                                              IndexBufferFormat;
    ID3D11InputLayout                                                        *InputLayout;
};

void dear_imgui_render(bool render_to_screen) {
    ImGui::Render();
    if (!render_to_screen) {
        return;
    }

    auto& io = ImGui::GetIO();
    auto draw_data = ImGui::GetDrawData();

    i32 width  = (i32)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    i32 height = (i32)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (height == 0 || width == 0) {
        return;
    }

    // Avoid rendering when minimized
    if (io.DisplaySize.x <= 0 || io.DisplaySize.y <= 0) {
        return;
    }

    // Create and grow vertex/index buffers if needed
    if ((imgui_dx.VB == nullptr) || (imgui_dx.VertexBufferSize < draw_data->TotalVtxCount)) {
        if (imgui_dx.VB != nullptr) {
            imgui_dx.VB->Release();
            imgui_dx.VB = nullptr;
        }
        imgui_dx.VertexBufferSize = draw_data->TotalVtxCount + 5000;
        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = (u32)imgui_dx.VertexBufferSize * sizeof(ImDrawVert);
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // todo(josh): should this CPU stuff be here?
        desc.MiscFlags = 0;
        auto result = imgui_dx.Device->CreateBuffer(&desc, nullptr, &imgui_dx.VB);
        assert(result == S_OK);
    }

    if ((imgui_dx.IB == nullptr) || (imgui_dx.IndexBufferSize < draw_data->TotalIdxCount)) {
        if (imgui_dx.IB != nullptr) {
            imgui_dx.IB->Release();
            imgui_dx.IB = nullptr;
        }
        imgui_dx.IndexBufferSize = draw_data->TotalIdxCount + 10000;
        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = (u32)imgui_dx.IndexBufferSize * sizeof(ImDrawVert);
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // todo(josh): should this CPU stuff be here?
        auto result = imgui_dx.Device->CreateBuffer(&desc, nullptr, &imgui_dx.IB);
        assert(result == S_OK);
    }


    // Upload vertex/index data into a single contiguous GPU buffer
    D3D11_MAPPED_SUBRESOURCE vtx_resource = {};
    auto result = imgui_dx.DeviceContext->Map((ID3D11Resource *)imgui_dx.VB, 0, D3D11_MAP_WRITE_DISCARD, 0, &vtx_resource);
    assert(result == S_OK);
    D3D11_MAPPED_SUBRESOURCE idx_resource = {};
    result = imgui_dx.DeviceContext->Map((ID3D11Resource *)imgui_dx.IB, 0, D3D11_MAP_WRITE_DISCARD, 0, &idx_resource);
    assert(result == S_OK);


    // auto vtx_dst = (ImDrawVert *)vtx_resource.pData;
    // auto idx_dst = (ImDrawIdx *) idx_resource.pData;
    // for list in mem.slice_ptr(draw_data.cmd_lists, int(draw_data.cmd_lists_count)) {
    //     mem.copy(vtx_dst, list.vtx_buffer.data, int(list.vtx_buffer.size) * sizeof(imgui.Draw_Vert));
    //     mem.copy(idx_dst, list.idx_buffer.data, int(list.idx_buffer.size) * sizeof(imgui.Draw_Idx));

    //     vtx_dst = mem.ptr_offset(vtx_dst, int(list.vtx_buffer.size));
    //     idx_dst = mem.ptr_offset(idx_dst, int(list.idx_buffer.size));
    // }
    // imgui_dx.DeviceContext->Unmap(imgui_dx.DeviceContext, cast(^ID3D11Resource)imgui_dx.VB, 0);
    // imgui_dx.DeviceContext->Unmap(imgui_dx.DeviceContext, cast(^ID3D11Resource)imgui_dx.IB, 0);

    ImDrawVert* vtx_dst = (ImDrawVert*)vtx_resource.pData;
    ImDrawIdx* idx_dst = (ImDrawIdx*)idx_resource.pData;
    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        vtx_dst += cmd_list->VtxBuffer.Size;
        idx_dst += cmd_list->IdxBuffer.Size;
    }
    imgui_dx.DeviceContext->Unmap(imgui_dx.VB, 0);
    imgui_dx.DeviceContext->Unmap(imgui_dx.IB, 0);

    // Setup orthographic projection matrix into our constant buffer
    // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
    {
        D3D11_MAPPED_SUBRESOURCE mapped_resource;
        if (imgui_dx.DeviceContext->Map(imgui_dx.VertexConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource) != S_OK)
            return;
        Imgui_CBuffer* constant_buffer = (Imgui_CBuffer*)mapped_resource.pData;
        float L = draw_data->DisplayPos.x;
        float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
        float T = draw_data->DisplayPos.y;
        float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
        float mvp[4][4] =
        {
            { 2.0f/(R-L),   0.0f,           0.0f,       0.0f },
            { 0.0f,         2.0f/(T-B),     0.0f,       0.0f },
            { 0.0f,         0.0f,           0.5f,       0.0f },
            { (R+L)/(L-R),  (T+B)/(B-T),    0.5f,       1.0f },
        };
        memcpy(&constant_buffer->mvp, mvp, sizeof(mvp));
        imgui_dx.DeviceContext->Unmap(imgui_dx.VertexConstantBuffer, 0);
    }

    // backup dx state
    BACKUP_DX11_STATE old = {};
    old.ScissorRectsCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    old.ViewportsCount    = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    imgui_dx.DeviceContext->RSGetScissorRects(&old.ScissorRectsCount, &old.ScissorRects[0]);
    imgui_dx.DeviceContext->RSGetViewports(&old.ViewportsCount, &old.Viewports[0]);
    imgui_dx.DeviceContext->RSGetState(&old.RS);
    imgui_dx.DeviceContext->OMGetBlendState(&old.BlendState, old.BlendFactor, &old.SampleMask);
    imgui_dx.DeviceContext->OMGetDepthStencilState(&old.DepthStencilState, &old.StencilRef);
    imgui_dx.DeviceContext->PSGetShaderResources(0, 1, &old.PSShaderResource);
    imgui_dx.DeviceContext->PSGetSamplers(0, 1, &old.PSSampler);
    old.PSInstancesCount = 256;
    old.VSInstancesCount = 256;
    old.GSInstancesCount = 256;
    imgui_dx.DeviceContext->PSGetShader(&old.PS, &old.PSInstances[0], &old.PSInstancesCount);
    imgui_dx.DeviceContext->VSGetShader(&old.VS, &old.VSInstances[0], &old.VSInstancesCount);
    imgui_dx.DeviceContext->VSGetConstantBuffers(0, 1, &old.VSConstantBuffer);
    imgui_dx.DeviceContext->GSGetShader(&old.GS, &old.GSInstances[0], &old.GSInstancesCount);

    imgui_dx.DeviceContext->IAGetPrimitiveTopology(&old.PrimitiveTopology);
    imgui_dx.DeviceContext->IAGetIndexBuffer(&old.IndexBuffer, &old.IndexBufferFormat, &old.IndexBufferOffset);
    imgui_dx.DeviceContext->IAGetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride, &old.VertexBufferOffset);
    imgui_dx.DeviceContext->IAGetInputLayout(&old.InputLayout);

    // Setup viewport
    D3D11_VIEWPORT vp = {};
    vp.Width    = io.DisplaySize.x;
    vp.Height   = io.DisplaySize.y;
    vp.MinDepth = 0;
    vp.MaxDepth = 1;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    imgui_dx.DeviceContext->RSSetViewports(1, &vp);

    // Setup shader and vertex buffers
    u32 stride = sizeof(ImDrawVert);
    u32 offset = 0;
    imgui_dx.DeviceContext->IASetInputLayout      (imgui_dx.InputLayout);
    imgui_dx.DeviceContext->IASetVertexBuffers    (0, 1, &imgui_dx.VB, &stride, &offset);
    imgui_dx.DeviceContext->IASetIndexBuffer      (imgui_dx.IB, sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
    imgui_dx.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    imgui_dx.DeviceContext->VSSetShader           (imgui_dx.VertexShader, nullptr, 0);
    imgui_dx.DeviceContext->VSSetConstantBuffers  (0, 1, &imgui_dx.VertexConstantBuffer);
    imgui_dx.DeviceContext->PSSetShader           (imgui_dx.PixelShader, nullptr, 0);
    imgui_dx.DeviceContext->PSSetSamplers         (0, 1, &imgui_dx.FontSampler);
    imgui_dx.DeviceContext->GSSetShader           (nullptr, nullptr, 0);
    imgui_dx.DeviceContext->HSSetShader           (nullptr, nullptr, 0); // In theory we should backup and restore this as well.. very infrequently used..
    imgui_dx.DeviceContext->DSSetShader           (nullptr, nullptr, 0); // In theory we should backup and restore this as well.. very infrequently used..
    imgui_dx.DeviceContext->CSSetShader           (nullptr, nullptr, 0); // In theory we should backup and restore this as well.. very infrequently used..

    // Setup blend state
    float blend_factor[4] = { 0, 0, 0, 0 };
    imgui_dx.DeviceContext->OMSetBlendState       (imgui_dx.BlendState, blend_factor, 0xffffffff);
    imgui_dx.DeviceContext->OMSetDepthStencilState(imgui_dx.DepthStencilState, 0);
    imgui_dx.DeviceContext->RSSetState            (imgui_dx.RasterizerState);


    // i32 vertex_buffer_offset = 0;
    // i32 index_buffer_offset  = 0;
    // for list in mem.slice_ptr(draw_data.cmd_lists, int(draw_data.cmd_lists_count)) {
    //     local_index_buffer_offset : u32 = 0;
    //     for cmd in mem.slice_ptr(list.cmd_buffer.data, int(list.cmd_buffer.size)) {
    //         texture := cast(^Texture)cmd.texture_id;
    //         bind_texture(texture, 0);
    //         defer bind_texture(nullptr, 0);

    //         r := D3D11_RECT{cast(i32)cmd.clip_rect.x, cast(i32)cmd.clip_rect.y, cast(i32)cmd.clip_rect.z, cast(i32)cmd.clip_rect.w};
    //         imgui_dx.DeviceContext->RSSetScissorRects(imgui_dx.DeviceContext, 1, &r);

    //         imgui_dx.DeviceContext->DrawIndexed(imgui_dx.DeviceContext, cmd.elem_count, cast(u32)local_index_buffer_offset + cast(u32)index_buffer_offset, cast(i32)vertex_buffer_offset);
    //         local_index_buffer_offset += cast(u32)cmd.elem_count;
    //     }
    //     vertex_buffer_offset += list.vtx_buffer.size;
    //     index_buffer_offset  += list.idx_buffer.size;
    // }

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    int global_idx_offset = 0;
    int global_vtx_offset = 0;
    ImVec2 clip_off = draw_data->DisplayPos;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            // Apply scissor/clipping rectangle
            const D3D11_RECT r = { (LONG)(pcmd->ClipRect.x - clip_off.x), (LONG)(pcmd->ClipRect.y - clip_off.y), (LONG)(pcmd->ClipRect.z - clip_off.x), (LONG)(pcmd->ClipRect.w - clip_off.y) };
            imgui_dx.DeviceContext->RSSetScissorRects(1, &r);

            // Bind texture, Draw
            Texture *texture = (Texture *)pcmd->TextureId;
            bind_texture(*texture, 0);
            imgui_dx.DeviceContext->DrawIndexed(pcmd->ElemCount, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset);
        }
        global_idx_offset += cmd_list->IdxBuffer.Size;
        global_vtx_offset += cmd_list->VtxBuffer.Size;
    }

    // Restore modified DX state
    imgui_dx.DeviceContext->RSSetScissorRects(old.ScissorRectsCount, &old.ScissorRects[0]);
    imgui_dx.DeviceContext->RSSetViewports(old.ViewportsCount, &old.Viewports[0]);
    imgui_dx.DeviceContext->RSSetState(old.RS);                                                                            if (old.RS != nullptr)                old.RS->Release();
    imgui_dx.DeviceContext->OMSetBlendState(old.BlendState, old.BlendFactor, old.SampleMask);                              if (old.BlendState != nullptr)        old.BlendState->Release();
    imgui_dx.DeviceContext->OMSetDepthStencilState(old.DepthStencilState, old.StencilRef);                                 if (old.DepthStencilState != nullptr) old.DepthStencilState->Release();
    imgui_dx.DeviceContext->PSSetShaderResources(0, 1, &old.PSShaderResource);                                             if (old.PSShaderResource != nullptr)  old.PSShaderResource->Release();
    imgui_dx.DeviceContext->PSSetSamplers(0, 1, &old.PSSampler);                                                           if (old.PSSampler != nullptr)         old.PSSampler->Release();
    imgui_dx.DeviceContext->PSSetShader(old.PS, &old.PSInstances[0], old.PSInstancesCount);                                if (old.PS != nullptr)                old.PS->Release();
    for (u32 i = 0; i < old.PSInstancesCount; i++)                                                                         if (old.PSInstances[i] != nullptr)    old.PSInstances[i]->Release();
    imgui_dx.DeviceContext->VSSetShader(old.VS, &old.VSInstances[0], old.VSInstancesCount);                                if (old.VS != nullptr)                old.VS->Release();
    imgui_dx.DeviceContext->VSSetConstantBuffers(0, 1, &old.VSConstantBuffer);                                             if (old.VSConstantBuffer != nullptr)  old.VSConstantBuffer->Release();
    imgui_dx.DeviceContext->GSSetShader(old.GS, &old.GSInstances[0], old.GSInstancesCount);                                if (old.GS != nullptr)                old.GS->Release();
    for (u32 i = 0; i < old.VSInstancesCount; i++)                                                                         if (old.VSInstances[i] != nullptr)    old.VSInstances[i]->Release();
    imgui_dx.DeviceContext->IASetPrimitiveTopology(old.PrimitiveTopology);
    imgui_dx.DeviceContext->IASetIndexBuffer(old.IndexBuffer, old.IndexBufferFormat, old.IndexBufferOffset);               if (old.IndexBuffer != nullptr)       old.IndexBuffer->Release();
    imgui_dx.DeviceContext->IASetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride, &old.VertexBufferOffset); if (old.VertexBuffer != nullptr)      old.VertexBuffer->Release();
    imgui_dx.DeviceContext->IASetInputLayout(old.InputLayout);                                                             if (old.InputLayout != nullptr)       old.InputLayout->Release();
}

/*

// todo(josh): this stuff

void    ImGui_ImplDX11_InvalidateDeviceObjects()
{
    if (!g_pd3dDevice)
        return;

    if (g_pFontSampler) { g_pFontSampler->Release(); g_pFontSampler = NULL; }
    if (g_pFontTextureView) { g_pFontTextureView->Release(); g_pFontTextureView = NULL; ImGui::GetIO().Fonts->TexID = NULL; } // We copied g_pFontTextureView to io.Fonts->TexID so let's clear that as well.
    if (g_pIB) { g_pIB->Release(); g_pIB = NULL; }
    if (g_pVB) { g_pVB->Release(); g_pVB = NULL; }

    if (g_pBlendState) { g_pBlendState->Release(); g_pBlendState = NULL; }
    if (g_pDepthStencilState) { g_pDepthStencilState->Release(); g_pDepthStencilState = NULL; }
    if (g_pRasterizerState) { g_pRasterizerState->Release(); g_pRasterizerState = NULL; }
    if (g_pPixelShader) { g_pPixelShader->Release(); g_pPixelShader = NULL; }
    if (g_pVertexConstantBuffer) { g_pVertexConstantBuffer->Release(); g_pVertexConstantBuffer = NULL; }
    if (g_pInputLayout) { g_pInputLayout->Release(); g_pInputLayout = NULL; }
    if (g_pVertexShader) { g_pVertexShader->Release(); g_pVertexShader = NULL; }
}

void ImGui_ImplDX11_Shutdown()
{
    ImGui_ImplDX11_InvalidateDeviceObjects();
    if (g_pFactory) { g_pFactory->Release(); g_pFactory = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
}

*/

#endif
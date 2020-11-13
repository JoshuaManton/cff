#include "render_backend.h"

Texture_Format_Info texture_format_infos[TF_COUNT];

#define SWAP_CHAIN_FORMAT      TF_R8G8B8A8_UINT
#define SWAP_CHAIN_FORMAT_SRGB TF_R8G8B8A8_UINT_SRGB

#ifdef RENDER_BACKEND_DX11
#include "dx.cpp"
#endif

void init_render_backend(Window *window) {
    texture_format_infos[TF_R8_UINT]            = {1,  1, false};
    texture_format_infos[TF_R32_INT]            = {4,  1, false};
    texture_format_infos[TF_R32_FLOAT]          = {4,  1, false};
    texture_format_infos[TF_R16G16B16A16_FLOAT] = {8,  4, false};
    texture_format_infos[TF_R32G32B32A32_FLOAT] = {16, 4, false};
    texture_format_infos[TF_R8G8B8A8_UINT]      = {4,  4, false};
    texture_format_infos[TF_R8G8B8A8_UINT_SRGB] = {4,  4, false};
    texture_format_infos[TF_DEPTH_STENCIL]      = {4,  2, true};

    // make sure all texture format infos are supplied
    for (int i = 0; i < ARRAYSIZE(texture_format_infos); i++) {
        if (texture_format_infos[i].pixel_size_in_bytes == 0) {
            if ((Texture_Format)i != TF_INVALID && (Texture_Format)i != TF_COUNT) {
                printf("Missing texture_format_info for %d\n", i);
                assert(false);
            }
        }
    }

    init_graphics_driver(window);
}

void create_color_and_depth_buffers(Texture_Description description, Texture *out_color_buffer, Texture *out_depth_buffer) {
    assert(out_color_buffer != nullptr);
    assert(out_depth_buffer != nullptr);

    description.render_target = true;
    *out_color_buffer = create_texture(description);

    description.wrap_mode = TWM_POINT_CLAMP;
    description.format = TF_DEPTH_STENCIL;
    *out_depth_buffer = create_texture(description);
}
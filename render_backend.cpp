#include "render_backend.h"

Texture_Format_Info texture_format_infos[TF_COUNT];

#define SWAP_CHAIN_FORMAT      TF_R8G8B8A8_UINT
#define SWAP_CHAIN_FORMAT_SRGB TF_R8G8B8A8_UINT_SRGB

#ifdef RENDER_BACKEND_DX11
#include "dx.cpp"
#endif

void init_render_backend(Window *window) {
    texture_format_infos[TF_R8G8B8A8_UINT]      = {4, 4, false};
    texture_format_infos[TF_R8G8B8A8_UINT_SRGB] = {4, 4, false};
    texture_format_infos[TF_DEPTH_STENCIL]      = {4, 4, true};

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
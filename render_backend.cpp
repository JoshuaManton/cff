#include "render_backend.h"

Texture_Format_Info texture_format_infos[TF_COUNT];

#ifdef RENDER_BACKEND_DX11
#include "dx.cpp"
#endif

void init_render_backend(Window *window) {
    texture_format_infos[TF_R8G8B8A8_UINT]      = {4, 4, false};
    texture_format_infos[TF_R8G8B8A8_UINT_SRGB] = {4, 4, false};
    texture_format_infos[TF_DEPTH_STENCIL]      = {4, 4, true};

    init_graphics_driver(window);
}
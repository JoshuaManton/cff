@rm *.obj
cl main.cpp math.cpp basic.cpp window.cpp renderer.cpp user32.lib d3d11.lib d3dcompiler.lib -DRENDER_BACKEND_DX11=1
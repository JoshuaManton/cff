cl /MP /Zi /Od /Fd /Iexternal main.cpp math.cpp basic.cpp window.cpp render_backend.cpp renderer.cpp assimp-vc141-mtd.lib user32.lib d3d11.lib d3dcompiler.lib -DRENDER_BACKEND_DX11=1 /EHsc /link /DEBUG
@rm *.obj
#include "types.hlsl"

TextureCube albedo_map : register(t0);

float4 main(PS_INPUT input) : SV_Target {
    float4 output_color = albedo_map.Sample(main_sampler, input.texcoord);
    output_color *= input.color * skybox_color;
    return output_color;
}
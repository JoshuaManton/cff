#include "types.hlsl"

Texture3D albedo_map    : register(t0);

float4 main(PS_INPUT input) : SV_Target {
    float4 output_color = float4(1, 1, 1, 1);
    if (has_albedo_map) {
        output_color = albedo_map.Sample(main_sampler, input.texcoord);
    }
    return output_color * input.color * model_color;
}
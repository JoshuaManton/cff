#include "types.hlsl"

Texture2D albedo_map : register(t0);

float4 main(PS_INPUT input) : SV_Target {
    float4 output_color = albedo_map.Sample(main_sampler, input.texcoord.xy);
    return output_color * input.color * model_color;
}
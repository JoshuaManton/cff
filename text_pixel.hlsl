#include "types.hlsl"

Texture2D albedo_map : register(t0);

float4 main(PS_INPUT input) : SV_Target {
    float value = albedo_map.Sample(main_sampler, input.texcoord.xy).r;
    float4 output_color = float4(1.0, 1.0, 1.0, value);
    return output_color * input.color * model_color;
}
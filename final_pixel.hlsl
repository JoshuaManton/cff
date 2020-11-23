#include "types.hlsl"

Texture2D albedo_map    : register(t0);
Texture2D bloom_map     : register(t1);

float4 main(PS_INPUT input) : SV_Target {
    float4 output_color = albedo_map.Sample(main_sampler, input.texcoord.xy);
    output_color.rgb += bloom_map.Sample(main_sampler, input.texcoord.xy).rgb;

    output_color.rgb = float3(1.0, 1.0, 1.0) - exp(-output_color.rgb * exposure);

    return output_color;
}
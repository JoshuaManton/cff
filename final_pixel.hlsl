#include "types.hlsl"

Texture2D albedo_map        : register(t0);
Texture2D bloom_map         : register(t1);
Texture2D ssr_map           : register(t2);

float4 main(PS_INPUT input) : SV_Target {
    float4 output_color = albedo_map.Sample(main_sampler, input.texcoord.xy);
    output_color.rgb += bloom_map.Sample(main_sampler, input.texcoord.xy).rgb;
    output_color.rgb += ssr_map.Sample(main_sampler, input.texcoord.xy).rgb;
    output_color.rgb *= exposure;
    output_color.rgb = output_color.rgb / (output_color.rgb + float3(1.0, 1.0, 1.0));
    return output_color;
}
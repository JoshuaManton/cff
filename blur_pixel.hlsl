#include "types.hlsl"

Texture2D albedo_map : register(t0);

float4 main(PS_INPUT input) : SV_Target {
    float3 output_color = float3(0, 0, 0);
    float2 texel_size = float2(1.0, 1.0) / buffer_dimensions;
    float2 texel_offset;
    if (horizontal == 1) {
        texel_offset = float2(texel_size.x, 0.0);
    }
    else {
        texel_offset = float2(0.0, texel_size.y);
    }

    const float RADIUS = 10;
    float total_weight = 0;
    for (float i = -RADIUS; i <= RADIUS; i++) {
        float factor = exp(-(i / RADIUS) * (i / RADIUS));
        total_weight += factor;
        output_color += albedo_map.Sample(main_sampler, input.texcoord.xy + texel_offset * i).rgb * factor;
    }
    return float4(output_color / total_weight, 1.0);
}
#include "types.hlsl"

Texture2D albedo_map : register(t0);

float gaussian(float t, float height, float width, float offset) {
    return height * exp(pow((-t - offset), 2) / pow(2 * width, 2));
}

float4 main(PS_INPUT input) : SV_Target {
    float2 texel_size = float2(1.0, 1.0) / buffer_dimensions;
    float2 texel_offset;
    if (horizontal == 1) {
        texel_offset = float2(texel_size.x, 0.0);
    }
    else {
        texel_offset = float2(0.0, texel_size.y);
    }

    float3 output_color = float3(0, 0, 0);
    if (blur_function == BLUR_FUNCTION_MARTIJN) {
        float total_weight = 0;
        // todo(josh): this will behave weirdly if blur_radius == 0
        for (int _i = -blur_radius; _i <= blur_radius; _i++) {
            float i = float(_i);
            float factor = exp(-(i / blur_radius) * (i / blur_radius));
            total_weight += factor;
            output_color += albedo_map.Sample(main_sampler, input.texcoord.xy + texel_offset * i).rgb * factor;
        }
        if (total_weight > 0) {
            output_color /= total_weight;
        }
    }
    else if (blur_function == BLUR_FUNCTION_GAUSSIAN) {
        for (int ii = 0; ii < blur_radius; ii++) {
            float i = float(ii);
            output_color += albedo_map.Sample(main_sampler, input.texcoord.xy + texel_offset *  i).rgb * gaussian(i, gaussian_height, 100, 0);
            output_color += albedo_map.Sample(main_sampler, input.texcoord.xy + texel_offset * -i).rgb * gaussian(i, gaussian_height, 100, 0);
        }
    }

    return float4(output_color, 1.0);
}
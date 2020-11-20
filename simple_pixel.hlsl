#include "types.hlsl"

float4 main(PS_INPUT input) : SV_Target {
    float4 output_color = float4(1, 1, 1, 1);
    output_color *= input.color;
    return output_color;
}
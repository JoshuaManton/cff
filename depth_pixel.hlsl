#include "types.hlsl"

float4 main(PS_INPUT input) : SV_Target {
    float z = input.position.z / input.position.w;
    return float4(z, z, z, 1.0);
}
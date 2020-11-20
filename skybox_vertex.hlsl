#include "types.hlsl"

PS_INPUT main(VS_INPUT input) {
    matrix mvp = mul(projection_matrix, mul(view_matrix, model_matrix));
    PS_INPUT v;
    v.position = mul(mvp, float4(input.position, 1.0)).xyww;
    v.texcoord = normalize(input.position);
    v.color = input.color;
    v.world_position = mul(model_matrix, float4(input.position, 1.0)).xyz;

    // todo(josh): fix normals for non-uniformly scaled objects
    float3 T = normalize(mul(model_matrix, float4(input.tangent,   0)).xyz);
    float3 B = normalize(mul(model_matrix, float4(input.bitangent, 0)).xyz);
    float3 N = normalize(mul(model_matrix, float4(input.normal,    0)).xyz);

    // this transpose is pretty lame
    v.tbn = transpose(matrix<float, 3, 3>(T, B, N));

    v.normal    = N;
    v.tangent   = T;
    v.bitangent = B;

    return v;
}
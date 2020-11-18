struct VS_INPUT {
    float3 position  : SV_POSITION;
    float3 texcoord  : TEXCOORD;
    float4 color     : COLOR;
    float3 normal    : NORMAL;
    float3 tangent   : TANGENT;
    float3 bitangent : BITANGENT;
};

struct PS_INPUT {
    float4 position         : SV_POSITION;
    float3 texcoord         : TEXCOORD;
    float4 color            : COLOR;
    float3 normal           : NORMAL;
    float3 tangent          : TANGENT;
    float3 bitangent        : BITANGENT;
    matrix<float, 3, 3> tbn : TBN;
    float3 world_position   : WORLDPOS;
};

cbuffer CBUFFER_PASS : register(b0) { // :PassCBufferSlot
    matrix view_matrix;
    matrix projection_matrix;
    float3 camera_position;
}

cbuffer CBUFFER_MODEL : register(b1) { // :ModelCBufferSlot
    matrix model_matrix;
    float4 model_color;
};

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
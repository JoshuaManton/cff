SamplerState main_sampler;

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
};

cbuffer CBUFFER_MODEL : register(b1) { // :ModelCBufferSlot
    matrix model_matrix;
    float4 model_color;
};

float4 main(PS_INPUT input) : SV_Target {
    float z = input.position.z / input.position.w;
    return float4(z, z, z, 1.0);
}
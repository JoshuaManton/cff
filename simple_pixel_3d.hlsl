SamplerState main_sampler;

Texture3D albedo_map    : register(t0);

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

cbuffer CBUFFER_MATERIAL : register(b2) { // :MaterialCBufferSlot
    int has_albedo_map;
};

float4 main(PS_INPUT input) : SV_Target {
    float4 output_color = float4(1, 1, 1, 1);
    if (has_albedo_map) {
        output_color = albedo_map.Sample(main_sampler, input.texcoord);
    }
    return output_color * input.color * model_color;
}
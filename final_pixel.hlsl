SamplerState main_sampler;

Texture2D albedo_map    : register(t0);
Texture2D bloom_map     : register(t1);

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
};

float4 main(PS_INPUT input) : SV_Target {
    float4 output_color = albedo_map.Sample(main_sampler, input.texcoord.xy);
    output_color.rgb += bloom_map.Sample(main_sampler, input.texcoord.xy).rgb;

    const float exposure = 0.25;
    output_color.rgb = float3(1.0, 1.0, 1.0) - exp(-output_color.rgb * exposure);

    return output_color;
}
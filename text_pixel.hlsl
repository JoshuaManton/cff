SamplerState main_sampler;

Texture2D albedo_map    : register(t0);

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

cbuffer CBUFFER_PASS : register(b0) {
    matrix view_matrix;
    matrix projection_matrix;
    float3 camera_position;
};

cbuffer CBUFFER_MODEL : register(b1) {
    matrix model_matrix;
    int has_albedo_map;
    int has_normal_map;
    int has_metallic_map;
    int has_roughness_map;
    int has_emission_map;
    int has_ao_map;
    float ambient;
    float metallic;
    float roughness;
    int visualize_normals;
    float pad0;
    float pad1;
};

float4 main(PS_INPUT input) : SV_Target {
    float4 output_color = float4(1, 1, 1, 1);
    if (has_albedo_map) {
        float value = albedo_map.Sample(main_sampler, input.texcoord.xy).r;
        output_color = float4(1.0, 1.0, 1.0, value);
    }
    else {
        output_color = float4(1, 0, 1, 1);
    }
    return output_color * input.color;
}
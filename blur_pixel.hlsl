SamplerState main_sampler;

Texture2D albedo_map : register(t0);

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

cbuffer CBUFFER_BLUR : register(b2) {
    int horizontal;
    float2 screen_dimensions;
}

float4 main(PS_INPUT input) : SV_Target {
    float3 output_color = float3(0, 0, 0);
    float2 texel_size = float2(1.0, 1.0) / screen_dimensions;
    float2 texel_offset;
    if (horizontal == 1) {
        texel_offset = float2(texel_size.x, 0.0);
    }
    else {
        texel_offset = float2(0.0, texel_size.y);
    }

    const float RADIUS = 50;
    float total_weight = 0;
    for (float i = -RADIUS; i <= RADIUS; i++) {
        float factor = exp(-(i / RADIUS) * (i / RADIUS));
        total_weight += factor;
        // float factor01 = (RADIUS - float(abs(i))) / float(RADIUS);
        output_color += albedo_map.Sample(main_sampler, input.texcoord.xy + texel_offset * i).rgb * factor;
    }
    return float4(output_color / total_weight, 1.0);
}
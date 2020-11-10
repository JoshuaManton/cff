SamplerState main_sampler;

Texture2D albedo : register(t0);
Texture2D normal : register(t1);

struct PS_INPUT {
    float4 position       : SV_POSITION;
    float3 texcoord       : TEXCOORD;
    float4 color          : COLOR;
    float3 world_position : WORLDPOS;
};

cbuffer CBUFFER_PASS : register(b0) {
    row_major matrix view_matrix;
    row_major matrix projection_matrix;
    float3 camera_position;
};

cbuffer CBUFFER_SPECIFIC : register(b1) {
    row_major matrix model_matrix;
    int has_albedo_map;
    int has_normal_map;
    int has_metallic_map;
    int has_roughness_map;
    int has_emission_map;
    int has_ao_map;
    float ambient;
    float metallic;
    float roughness;
    float pad0;
    float pad1;
    float pad2;
};

float4 main(PS_INPUT input) : SV_Target {
    float4 color = float4(1, 1, 1, 1);
    if (has_albedo_map) {
        color = albedo.Sample(main_sampler, input.texcoord.xy);
    }

    if (has_ao_map) {
        color *= albedo.Sample(main_sampler, input.texcoord.xy).r;
    }

    color.rgb *= 1.0f-smoothstep(0.0f, 35.0f, length(camera_position - input.world_position));
    return color;
}
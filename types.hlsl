SamplerState main_sampler;

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

struct PS_OUTPUT {
    float4 color       : SV_Target0;
    float4 bloom_color : SV_Target1;
    float4 depth       : SV_Target2;
};

cbuffer CBUFFER_PASS : register(b0) {
    matrix view_matrix;
    matrix projection_matrix;
    float3 camera_position;
};

cbuffer CBUFFER_MODEL : register(b1) {
    matrix model_matrix;
    float4 model_color;
};

cbuffer CBUFFER_MATERIAL : register(b2) {
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

#define MAX_POINT_LIGHTS 16 // :MaxPointLights
cbuffer CBUFFER_LIGHTING : register(b3) {
    float4 point_light_positions[MAX_POINT_LIGHTS];
    float4 point_light_colors[MAX_POINT_LIGHTS];
    int num_point_lights;
    matrix sun_transform;
    float3 sun_direction;
    float3 sun_color;
    float fog_y_level;
    float fog_density;
    float3 fog_base_color;
    int has_skybox_map;
    float4 skybox_color;
};

cbuffer CBUFFER_BLUR : register(b4) {
    int horizontal;
    float2 buffer_dimensions;
}

cbuffer CBUFFER_FINAL : register(b5) {
    float exposure;
}
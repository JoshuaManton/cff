SamplerState main_sampler;

TextureCube albedo_map : register(t0);

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

#define MAX_POINT_LIGHTS 16 // :MaxPointLights
cbuffer CBUFFER_LIGHTING : register(b3) { // :LightingCBufferSlot
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

float4 main(PS_INPUT input) : SV_Target {
    float4 output_color = albedo_map.Sample(main_sampler, input.texcoord);
    output_color *= input.color * skybox_color;
    return output_color;
}
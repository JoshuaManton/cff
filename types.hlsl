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
    float4 albedo      : SV_Target2;
    float4 position    : SV_Target3;
    float4 normal      : SV_Target4;
    float4 material    : SV_Target5;
};

cbuffer CBUFFER_PASS : register(b0) {
    float2 screen_dimensions;
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
    float pad0;
    float pad1;
    float pad2;
};

#define NUM_SHADOW_MAPS 4 // :NumShadowMaps
#define SHADOW_MAP_DIM 2048 // :ShadowMapDim
#define MAX_POINT_LIGHTS 16 // :MaxPointLights
cbuffer CBUFFER_LIGHTING : register(b3) {
    float4 point_light_positions[MAX_POINT_LIGHTS];
    float4 point_light_colors[MAX_POINT_LIGHTS];
    int num_point_lights;
    float3 sun_direction;
    matrix sun_transforms[NUM_SHADOW_MAPS];
    float4 cascade_distances;
    int visualize_cascades;
    float3 sun_color;
    int do_fog;
    float3 fog_color;
    float fog_y_level;
    float fog_density;
    float bloom_slope;
    float bloom_threshold;
    float ambient_modifier;
    int has_skybox_map;
    float4 skybox_color;
};

cbuffer CBUFFER_BLUR : register(b4) {
    int horizontal;
    float2 buffer_dimensions;
    float blur_radius; // todo(josh): I don't remember what space this radius is in. investigate and make sure it makes sense
    int blur_function;
    float gaussian_height;
};

// :BlurFunction
#define BLUR_FUNCTION_MARTIJN 0
#define BLUR_FUNCTION_GAUSSIAN 1

cbuffer CBUFFER_FINAL : register(b5) {
    float exposure;
};

cbuffer CBUFFER_SSR : register(b6) {
    float3 scene_camera_position;
    matrix camera_matrix;
    matrix inverse_camera_matrix;
};
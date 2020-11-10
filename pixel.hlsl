SamplerState main_sampler;

Texture2D albedo_map : register(t0);
Texture2D normal_map : register(t1);

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
    float pad0;
    float pad1;
    float pad2;
};

#define MAX_POINT_LIGHTS 16 // :MaxPointLights
cbuffer CBUFFER_LIGHTING : register(b2) {
    float4 point_light_positions[MAX_POINT_LIGHTS];
    float4 point_light_colors[MAX_POINT_LIGHTS];
    int num_point_lights;
};

#define PI 3.14159265359

float3 fresnel_schlick(float cosTheta, float3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float distribution_ggx(float3 N, float3 H, float roughness) {
    float a      = roughness*roughness;
    float NdotH  = max(dot(N, H), 0.001);
    float NdotH2 = NdotH*NdotH;

    float num   = a;
    float denom = (NdotH2 * (a - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float geometry_schlick_ggx(float NdotV, float roughness, int analytic) {
    // todo(josh): (roughness + 1) should only be used for analytic light sources, not IBL
    // "if applied to image-based lighting, the results at glancing angles will be much too dark"
    // page 3
    // https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf

    float k;
    if (analytic == 1) {
        k = pow((roughness + 1.0), 2.0) * 0.125;
    }
    else {
        k = pow(roughness, 2.0) * 0.5;
    }

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}
float geometry_smith(float3 N, float3 V, float3 L, float roughness, int analytic) {
    float NdotV = max(dot(N, V), 0.001);
    float NdotL = max(dot(N, L), 0.001);
    float ggx2  = geometry_schlick_ggx(NdotV, roughness, analytic);
    float ggx1  = geometry_schlick_ggx(NdotL, roughness, analytic);

    return ggx1 * ggx2;
}

float3 calculate_light(float3 albedo, float metallic, float roughness, float3 N, float3 V, float3 L, float3 incoming_radiance, int analytic) {
    float3 H = normalize(V + L);

    // todo(josh): no need to do this for each light, should be constant for a given draw call
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metallic);

    // cook-torrance brdf
    float  NDF = distribution_ggx(N, H, roughness);
    float  G   = geometry_smith(N, V, L, roughness, analytic);
    float3 F   = fresnel_schlick(saturate(dot(H, V)), F0);

    float3 kS = F;
    float3 kD = float3(1.0, 1.0, 1.0) - kS;
    kD *= 1.0 - metallic;

    float3 numerator  = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.001) * max(dot(N, L), 0.001);
    float3 specular   = numerator / max(denominator, 0.001);

    // add to outgoing incoming_radiance Lo
    float NdotL = max(dot(N, L), 0.001);
    return (kD * albedo / PI + specular) * incoming_radiance * NdotL;
}

float4 main(PS_INPUT input) : SV_Target {
    float3 N = normalize(input.normal);
    if (has_normal_map == 1) {
        N = normal_map.Sample(main_sampler, input.texcoord.xy).rgb;
        N = N * 2.0 - 1.0;
        N = normalize(mul(input.tbn, N));
    }

    float3 V = normalize(camera_position - input.world_position);

    float4 output_color = float4(1, 1, 1, 1);
    if (has_albedo_map) {
        output_color = albedo_map.Sample(main_sampler, input.texcoord.xy);
    }

    if (has_ao_map) {
        output_color *= albedo_map.Sample(main_sampler, input.texcoord.xy).r;
    }

    float3 albedo = output_color.rgb;

    for (int point_light_index = 0; point_light_index < num_point_lights; point_light_index++) {
        float3 light_position = point_light_positions[point_light_index].xyz;
        float3 light_color    = point_light_colors[point_light_index].xyz;

        float3 offset_to_light = light_position - input.world_position;
        float distance_to_light = length(offset_to_light);
        float attenuation = 1.0 / (distance_to_light * distance_to_light);
        float3 L = normalize(offset_to_light);
        output_color.rgb += calculate_light(albedo, metallic, roughness, N, V, L, light_color * attenuation, 1);
    }

    output_color.rgb *= 1.0f-smoothstep(-5, 35.0f, length(camera_position - input.world_position)); // note(josh): garbage depth-darkness thing
    return output_color;
}
SamplerState main_sampler;

Texture2D albedo_map    : register(t0);
Texture2D normal_map    : register(t1);
Texture2D metallic_map  : register(t2);
Texture2D roughness_map : register(t3);
Texture2D emission_map  : register(t4);
Texture2D ao_map        : register(t5);
Texture2D shadow_map    : register(t6);

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

#define MAX_POINT_LIGHTS 16 // :MaxPointLights
cbuffer CBUFFER_LIGHTING : register(b2) {
    float4 point_light_positions[MAX_POINT_LIGHTS];
    float4 point_light_colors[MAX_POINT_LIGHTS];
    int num_point_lights;
    matrix sun_transform;
    float3 sun_direction;
    float3 sun_color;
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
    // node(josh): (roughness + 1) should only be used for analytic light sources, not IBL
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

    float NdotL = max(dot(N, L), 0.001);
    return (kD * albedo / PI + specular) * incoming_radiance * NdotL;
}

float calculate_shadow(Texture2D shadow_map_texture, row_major matrix sun_matrix, float3 world_pos, float3 N) {
    float4 frag_position_light_space = mul(sun_matrix, float4(world_pos, 1.0));
    float3 proj_coords = frag_position_light_space.xyz / frag_position_light_space.w; // todo(josh): check for divide by zero?
    proj_coords.xy = proj_coords.xy * 0.5 + 0.5;
    proj_coords.y = 1.0 - proj_coords.y;
    if (proj_coords.z > 1.0) {
        proj_coords.z = 1.0;
    }

    float dot_to_sun = clamp(dot(N, -sun_direction), 0, 1);
    float bias = max(0.01 * (1.0 - dot_to_sun), 0.001);
    // float bias = 0.01;

    float2 texel_size = 1.0 / 2048;
    float shadow = 0;
#if 0
    shadow = shadow_map_texture.Sample(main_sampler, proj_coords.xy).r + bias < proj_coords.z ? 1.0 : 0.0;
#elif 1
    shadow += shadow_map_texture.Sample(main_sampler, proj_coords.xy + float2(-1, -1) * texel_size).r + bias < proj_coords.z ? 1.0 : 0.0;
    shadow += shadow_map_texture.Sample(main_sampler, proj_coords.xy + float2( 0, -1) * texel_size).r + bias < proj_coords.z ? 1.0 : 0.0;
    shadow += shadow_map_texture.Sample(main_sampler, proj_coords.xy + float2( 1, -1) * texel_size).r + bias < proj_coords.z ? 1.0 : 0.0;
    shadow += shadow_map_texture.Sample(main_sampler, proj_coords.xy + float2(-1,  0) * texel_size).r + bias < proj_coords.z ? 1.0 : 0.0;
    shadow += shadow_map_texture.Sample(main_sampler, proj_coords.xy + float2( 0,  0) * texel_size).r + bias < proj_coords.z ? 1.0 : 0.0;
    shadow += shadow_map_texture.Sample(main_sampler, proj_coords.xy + float2( 1,  0) * texel_size).r + bias < proj_coords.z ? 1.0 : 0.0;
    shadow += shadow_map_texture.Sample(main_sampler, proj_coords.xy + float2(-1,  1) * texel_size).r + bias < proj_coords.z ? 1.0 : 0.0;
    shadow += shadow_map_texture.Sample(main_sampler, proj_coords.xy + float2( 0,  1) * texel_size).r + bias < proj_coords.z ? 1.0 : 0.0;
    shadow += shadow_map_texture.Sample(main_sampler, proj_coords.xy + float2( 1,  1) * texel_size).r + bias < proj_coords.z ? 1.0 : 0.0;
    shadow /= 9.0;
#else
    for (int x = -2; x <= 2; x += 1) {
        for (int y = -2; y <= 2; y += 1) {
            float pcf_depth = shadow_map_texture.Sample(main_sampler, proj_coords.xy + float2(x, y) * texel_size).r;
            shadow += pcf_depth + bias < proj_coords.z ? 1.0 : 0.0;
        }
    }
    shadow /= 25.0;
#endif
    return shadow;
}

PS_OUTPUT main(PS_INPUT input) {
    float3 N = normalize(input.normal);
    if (has_normal_map == 1) {
        N = normal_map.Sample(main_sampler, input.texcoord.xy).rgb;
        N = N * 2.0 - 1.0;
        N = normalize(mul(input.tbn, N));
    }
    if (visualize_normals == 1) {
        PS_OUTPUT output;
        output.color = float4(N * 0.5 + 0.5, 1.0);
        output.bloom_color = float4(0, 0, 0, 0);
        return output;
    }

    float3 V = normalize(camera_position - input.world_position);

    float4 output_color = float4(1, 1, 1, 1);
    if (has_albedo_map) {
        output_color = albedo_map.Sample(main_sampler, input.texcoord.xy);
    }

    float3 albedo = output_color.rgb;

    if (has_ao_map) {
        output_color.rgb *= ao_map.Sample(main_sampler, input.texcoord.xy).r;
    }

    output_color.rgb *= ambient;

    for (int point_light_index = 0; point_light_index < num_point_lights; point_light_index++) {
        float3 light_position = point_light_positions[point_light_index].xyz;
        float3 light_color    = point_light_colors[point_light_index].rgb;

        float3 offset_to_light = light_position - input.world_position;
        float distance_to_light = length(offset_to_light);
        float attenuation = 1.0 / (distance_to_light * distance_to_light);
        float3 L = normalize(offset_to_light);
        output_color.rgb += calculate_light(albedo, metallic, roughness, N, V, L, light_color * attenuation, 1);
    }

    if (length(sun_direction) > 0) {
        float shadow = calculate_shadow(shadow_map, sun_transform, input.world_position, N);
        output_color.rgb += calculate_light(albedo, metallic, roughness, N, V, -sun_direction, sun_color, 1) * (1.0f - shadow);
    }

    // output_color.rgb *= 1.0f-smoothstep(-5, 35.0f, length(camera_position - input.world_position)); // note(josh): garbage depth-darkness thing

    PS_OUTPUT output;
    output.color = output_color;
    output.bloom_color = float4(0, 0, 0, 0);
    float color_magnitude = length(output.color.rgb);
    const float BLOOM_THRESHOLD = 10.0;
    if (color_magnitude > BLOOM_THRESHOLD) {
        const float SLOPE = 0.5;
        output.bloom_color.rgb = (output.color.rgb - (normalize(output.color.rgb) * BLOOM_THRESHOLD)) * SLOPE;
        output.bloom_color.a = output.color.a;
    }
    return output;
}
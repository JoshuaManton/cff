#include "types.hlsl"
#include "pbr.hlsl"

Texture2D   albedo_map    : register(t0);
Texture2D   normal_map    : register(t1);
Texture2D   metallic_map  : register(t2);
Texture2D   roughness_map : register(t3);
Texture2D   emission_map  : register(t4);
Texture2D   ao_map        : register(t5);
Texture2D   shadow_map    : register(t6);
Texture2D   depth_prepass : register(t7);
TextureCube skybox_map    : register(t8);

int sun_can_see_point(float3 position, row_major matrix sun_matrix, Texture2D shadow_map_texture) {
    float4 position_sun_space = mul(sun_matrix, float4(position, 1.0));
    float3 proj_coords = position_sun_space.xyz / position_sun_space.w; // todo(josh): check for divide by zero?
    proj_coords.xy = proj_coords.xy * 0.5 + 0.5;
    proj_coords.y = 1.0 - proj_coords.y;
    if (proj_coords.z > 1.0) {
        proj_coords.z = 1.0;
    }
    float sun_depth = shadow_map_texture.Sample(main_sampler, proj_coords.xy).r;
    if (sun_depth < (proj_coords.z-0.001)) {
        return 0;
    }
    return 1;
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

    float distance_to_pixel_position = length(camera_position - input.world_position);
    float3 direction_to_camera = (camera_position - input.world_position) / distance_to_pixel_position;

    float4 output_color = float4(1, 1, 1, 1);
    if (has_albedo_map) {
        output_color = albedo_map.Sample(main_sampler, input.texcoord.xy);
    }
    output_color.rgb *= model_color.rgb * input.color.rgb;

    float3 albedo = output_color.rgb;
    float ao = 1.0;
    if (has_ao_map) {
        ao = ao_map.Sample(main_sampler, input.texcoord.xy).r;
    }
    output_color.rgb *= ambient * ao;

    float4 normal_as_color = float4(N * 0.5 + 0.5, 1.0);

    // todo(josh): delete this if we continue with deferred rendering
    if (visualize_normals == 1) {
        PS_OUTPUT output;
        output.color = normal_as_color;
        output.bloom_color = float4(0, 0, 0, 0);
        output.position = float4(input.world_position, 1.0);
        output.normal = normal_as_color;
        output.material = float4(metallic, roughness, ao, 1.0);
        return output;
    }

    for (int point_light_index = 0; point_light_index < num_point_lights; point_light_index++) {
        float3 light_position = point_light_positions[point_light_index].xyz;
        float3 light_color    = point_light_colors[point_light_index].rgb;

        float3 offset_to_light = light_position - input.world_position;
        float distance_to_light = length(offset_to_light);
        float attenuation = 1.0 / (distance_to_light * distance_to_light);
        float3 direction_to_light = normalize(offset_to_light);
        output_color.rgb += calculate_light(albedo, metallic, roughness, N, direction_to_camera, direction_to_light, light_color * attenuation, 1);
    }

    if (length(sun_direction) > 0) {
        float shadow = calculate_shadow(shadow_map, sun_transform, input.world_position, N);
        output_color.rgb += calculate_light(albedo, metallic, roughness, N, direction_to_camera, -sun_direction, sun_color, 1) * (1.0f - shadow);
    }

    if (has_skybox_map) {
        // todo(josh): figure out what to do with skybox reflections
        // float3 reflected_direction = normalize(reflect(-direction_to_camera, N));
        // float4 skybox_sample_color = skybox_map.Sample(main_sampler, reflected_direction) * skybox_color;
        // output_color.rgb += calculate_light(albedo, metallic, roughness, N, direction_to_camera, reflected_direction, skybox_sample_color.rgb, 0);
    }

    if (has_emission_map) {
        float4 emission_sample = emission_map.Sample(main_sampler, input.texcoord.xy);
        output_color.rgb += emission_sample.rgb;
    }

    float total_density = 0;
    const float STEP_SIZE = 0.1;
    const int MAX_STEPS = 200;
    for (int step = 0; step < MAX_STEPS; step++) {
        float3 ray_position = camera_position - direction_to_camera * STEP_SIZE * step;
        float ray_distance = length(camera_position - ray_position);
        if (ray_distance < distance_to_pixel_position && sun_can_see_point(ray_position, sun_transform, shadow_map)) {
            float fog_amount = 1.0 - exp(-fog_density * (1.0 / (max(1.0, input.world_position.y - fog_y_level))));
            total_density += fog_amount * STEP_SIZE;
        }
    }
    output_color.rgb = lerp(output_color.rgb, fog_base_color, saturate(total_density));


    PS_OUTPUT output;
    output.color = output_color;
    output.bloom_color = float4(0, 0, 0, output.color.a);
    output.position = float4(input.world_position, 1.0);
    output.normal = normal_as_color;
    output.material = float4(metallic, roughness, ao, 1.0);
    float color_magnitude = length(output.color.rgb);
    const float BLOOM_THRESHOLD = 10.0;
    if (color_magnitude > BLOOM_THRESHOLD) {
        const float SLOPE = 0.5;
        output.bloom_color.rgb = (output.color.rgb - (normalize(output.color.rgb) * BLOOM_THRESHOLD)) * SLOPE;
        output.bloom_color.a = output.color.a;
    }
    return output;
}
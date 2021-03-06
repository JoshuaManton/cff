#include "types.hlsl"
#include "pbr.hlsl"

Texture2D   albedo_map    : register(t0);
Texture2D   normal_map    : register(t1);
Texture2D   metallic_map  : register(t2);
Texture2D   roughness_map : register(t3);
Texture2D   emission_map  : register(t4);
Texture2D   ao_map        : register(t5);
TextureCube skybox_map    : register(t6);
Texture2D   shadow_map1   : register(t7);
Texture2D   shadow_map2   : register(t8);
Texture2D   shadow_map3   : register(t9);
Texture2D   shadow_map4   : register(t10);

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

    float2 texel_size = 1.0 / SHADOW_MAP_DIM;
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
    float4 normal_as_color = float4(N * 0.5 + 0.5, 1.0);

    float distance_to_pixel_position = length(camera_position - input.world_position);
    float3 direction_to_camera = (camera_position - input.world_position) / distance_to_pixel_position;

    float4 output_color = model_color * input.color;
    if (has_albedo_map) {
        output_color *= albedo_map.Sample(main_sampler, input.texcoord.xy);
    }

    float3 albedo = output_color.rgb;
    float ao = 1.0;
    if (has_ao_map) {
        ao = ao_map.Sample(main_sampler, input.texcoord.xy).r;
    }
    output_color.rgb *= (ambient * ambient_modifier) * ao;

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
        float shadow = 0;
        int cascade_index = -1;

        #if 1 // note(josh): interpolate between cascades
             if (distance_to_pixel_position < cascade_distances.x) { float cascade_size = cascade_distances.x;                       float distance_t = (distance_to_pixel_position                      ) / cascade_size; cascade_index = 0; shadow = lerp(calculate_shadow(shadow_map1, sun_transforms[0], input.world_position, N), calculate_shadow(shadow_map2, sun_transforms[1], input.world_position, N), smoothstep(0.75, 1, distance_t)); }
        else if (distance_to_pixel_position < cascade_distances.y) { float cascade_size = cascade_distances.y - cascade_distances.x; float distance_t = (distance_to_pixel_position - cascade_distances.x) / cascade_size; cascade_index = 1; shadow = lerp(calculate_shadow(shadow_map2, sun_transforms[1], input.world_position, N), calculate_shadow(shadow_map3, sun_transforms[2], input.world_position, N), smoothstep(0.75, 1, distance_t)); }
        else if (distance_to_pixel_position < cascade_distances.z) { float cascade_size = cascade_distances.z - cascade_distances.y; float distance_t = (distance_to_pixel_position - cascade_distances.y) / cascade_size; cascade_index = 2; shadow = lerp(calculate_shadow(shadow_map3, sun_transforms[2], input.world_position, N), calculate_shadow(shadow_map4, sun_transforms[3], input.world_position, N), smoothstep(0.75, 1, distance_t)); }
        else if (distance_to_pixel_position < cascade_distances.w) { float cascade_size = cascade_distances.w - cascade_distances.z; float distance_t = (distance_to_pixel_position - cascade_distances.z) / cascade_size; cascade_index = 3; shadow = lerp(calculate_shadow(shadow_map4, sun_transforms[3], input.world_position, N), 0,                                                                         smoothstep(0.75, 1, distance_t)); }
        #else
             if (distance_to_pixel_position < cascade_distances.x) { cascade_index = 0; shadow = calculate_shadow(shadow_map1, sun_transforms[0], input.world_position, N); }
        else if (distance_to_pixel_position < cascade_distances.y) { cascade_index = 1; shadow = calculate_shadow(shadow_map2, sun_transforms[1], input.world_position, N); }
        else if (distance_to_pixel_position < cascade_distances.z) { cascade_index = 2; shadow = calculate_shadow(shadow_map3, sun_transforms[2], input.world_position, N); }
        else if (distance_to_pixel_position < cascade_distances.w) { cascade_index = 3; shadow = calculate_shadow(shadow_map4, sun_transforms[3], input.world_position, N); }
        #endif

        output_color.rgb += calculate_light(albedo, metallic, roughness, N, direction_to_camera, -sun_direction, sun_color, 1) * (1.0f - shadow);

        if (visualize_cascades) {
            if (cascade_index == -1) {
                output_color.rb += 1; // magenta for no cascade
            }
            else if (cascade_index == 0) {
                output_color.r += 1;
            }
            else if (cascade_index == 1) {
                output_color.g += 1;
            }
            else if (cascade_index == 2) {
                output_color.b += 1;
            }
            else if (cascade_index == 3) {
                output_color.rg += 1;
            }
        }
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

    if (do_fog) {
        float total_density = 0;
        const float STEP_SIZE = 0.1;
        const int MAX_DISTANCE = 20;
        float ray_distance = 0;
        while (ray_distance < MAX_DISTANCE) {
            float3 ray_position = camera_position - direction_to_camera * ray_distance;
            // todo(josh): use the correct sun cascade here
            float fog_amount = 1.0 - exp(-fog_density * (1.0 / (max(1.0, input.world_position.y - fog_y_level))));
            float factor = (ray_distance < distance_to_pixel_position && sun_can_see_point(ray_position, sun_transforms[3], shadow_map4)) ? 1 : 0;
            total_density += fog_amount * STEP_SIZE * factor;
            ray_distance += STEP_SIZE;
        }
        output_color.rgb = lerp(output_color.rgb, fog_color, saturate(total_density));
    }



    PS_OUTPUT output;
    output.color = output_color;
    output.albedo = float4(albedo, 1.0);
    output.bloom_color = float4(0, 0, 0, output.color.a);
    output.position = float4(input.world_position, 1.0);
    output.normal = normal_as_color;
    output.material = float4(metallic, roughness, ao, 1.0);
    float color_magnitude = dot(output.color.rgb, float3(0.2, 0.7, 0.1)); // todo(josh): @CorrectBrightness (0.2, 0.7, 0.1) are not exact. martijn: "if you want the exact ones, look at wikipedia at the Y component of the RGB primaries of the sRGB color space"
    if (color_magnitude > bloom_threshold) {
        output.bloom_color.rgb = (output.color.rgb - (normalize(output.color.rgb) * bloom_threshold)) * bloom_slope;
        output.bloom_color.a = output.color.a;
    }
    return output;
}
#include "types.hlsl"
#include "pbr.hlsl"

Texture2D scene_map           : register(t0);
Texture2D normal_map          : register(t1);
Texture2D positions_map       : register(t2);
Texture2D metal_roughness_map : register(t3);

float3 ray_march(float3 surface_color, float3 surface_pixel_position, float3 surface_normal, float3 dir_to_pixel, float3 ray_direction, float metallic, float roughness) {
    float step_distance = 0;
    const float STEP_SIZE = 0.1;
    float3 reflected_color = float3(0, 0, 0);
    [loop]
    while (step_distance < 5) {
        step_distance += STEP_SIZE;
        float3 ray_position = surface_pixel_position + ray_direction * step_distance;
        float4 ray_position_viewport_space = mul(camera_matrix, float4(ray_position, 1.0));
        ray_position_viewport_space /= ray_position_viewport_space.w;
        float2 ray_position_uv = ray_position_viewport_space.xy * 0.5 + 0.5;
        ray_position_uv.y = 1.0 - ray_position_uv.y;
        float3 ray_position_position_sample = positions_map.Sample(main_sampler, ray_position_uv).rgb;
        float4 ray_position_position_sample_viewport_space = mul(camera_matrix, float4(ray_position_position_sample, 1.0));
        ray_position_position_sample_viewport_space /= ray_position_position_sample_viewport_space.w;

        if (ray_position_viewport_space.z > ray_position_position_sample_viewport_space.z &&
            (length(ray_position - ray_position_position_sample) < (STEP_SIZE * 1.5)) &&
            ray_position_viewport_space.x < 1 &&
            ray_position_viewport_space.x > -1 &&
            ray_position_viewport_space.y < 1 &&
            ray_position_viewport_space.y > -1) {

            // todo(josh): maybe binary search to get closer to the surface that the ray hit?

            float3 ray_position_screen_sample = scene_map.Sample(main_sampler, ray_position_uv).rgb;

            float3 offset_to_light = ray_position - surface_pixel_position;
            float distance_to_light = length(offset_to_light);
            // todo(josh): this attenuation is to make the edges of the screen reflection a little softer but it looks bad so figure out something better
            float attentuation_fade_term = 1.0 - (saturate(length(ray_position_position_sample_viewport_space.xy)));
            // todo(josh): 0 or 1 for analytic parameter?
            float attentuation = saturate(1.0 / ((distance_to_light * distance_to_light) + 1));
            reflected_color.rgb = attentuation_fade_term * calculate_light(surface_color, metallic, roughness, surface_normal, -dir_to_pixel, ray_direction, ray_position_screen_sample * attentuation, 1);
            break;
        }
    }
    return reflected_color;
}

float3 arbitrary_perpendicular(float3 a) {
    float3 b = float3(1, 0, 0);
    if (abs(dot(a, b)) == 1) {
        b = float3(0, 1, 0);
    }
    float3 d = cross(a, b);
    return d;
}

float4 main(PS_INPUT input) : SV_Target0 {
    float2 screen_uv = float2(input.position.xy / screen_dimensions);
    float3 surface_color = scene_map.Sample(main_sampler, screen_uv).xyz;
    float3 surface_normal = normal_map.Sample(main_sampler, screen_uv).xyz;
    surface_normal = surface_normal * 2 - 1;
    float3 surface_pixel_position = positions_map.Sample(main_sampler, screen_uv).xyz;
    float2 metal_roughness_sample = metal_roughness_map.Sample(main_sampler, screen_uv).xy;
    float3 dir_to_pixel = normalize(surface_pixel_position - scene_camera_position);
    float3 reflected_direction = normalize(reflect(dir_to_pixel, surface_normal));
    float3 reflected_color = float3(0, 0, 0);
    float3 perp_to_normal = arbitrary_perpendicular(surface_normal);
    float3 perp2 = cross(surface_normal, perp_to_normal);
    matrix<float, 3, 3> surface_transform = transpose(matrix<float, 3, 3>(perp_to_normal, perp2, surface_normal));
    reflected_color += ray_march(surface_color, surface_pixel_position, surface_normal, dir_to_pixel, reflected_direction, metal_roughness_sample.x, metal_roughness_sample.y);
    // const float RESOLUTION_RADIANS = 0.3;
    // const float FACTOR = RESOLUTION_RADIANS / (2 * PI);
    // for (float z_theta = 0; z_theta < (PI * 0.5); z_theta += RESOLUTION_RADIANS) {
    //     for (float xy_theta = 0; xy_theta < (PI * 2); xy_theta += RESOLUTION_RADIANS) {
    //         float3 bounce_direction = mul(surface_transform, normalize(float3(sin(xy_theta) - sin(z_theta), cos(xy_theta) - sin(z_theta), sin(z_theta))));
    //         reflected_color += FACTOR * ray_march(surface_color, surface_pixel_position, surface_normal, dir_to_pixel, bounce_direction, metal_roughness_sample.x, metal_roughness_sample.y);
    //     }
    // }

    // float4 main_scene_sample = scene_map.Sample(main_sampler, screen_uv);
    // main_scene_sample.rgb += reflected_color;
    return float4(reflected_color, 1.0);
    // float4 scene_color = scene_map.Sample(main_sampler, input.texcoord.xy);
    // return float4(scene_color.r, 0, 0, 1);
}
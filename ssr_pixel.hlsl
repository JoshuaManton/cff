#include "types.hlsl"
#include "pbr.hlsl"

Texture2D scene_map           : register(t0);
Texture2D normal_map          : register(t1);
Texture2D positions_map       : register(t2);
Texture2D metal_roughness_map : register(t3);

float4 main(PS_INPUT input) : SV_Target0 {
    float2 screen_uv = float2(input.position.xy / screen_dimensions);
    float3 base_color = scene_map.Sample(main_sampler, screen_uv).xyz;
    float3 scene_pixel_normal = normal_map.Sample(main_sampler, screen_uv).xyz;
    scene_pixel_normal = scene_pixel_normal * 2 - 1;
    float3 scene_pixel_position = positions_map.Sample(main_sampler, screen_uv).xyz;
    float2 metal_roughness_sample = metal_roughness_map.Sample(main_sampler, screen_uv).xy;
    float3 dir_to_pixel = normalize(scene_pixel_position - scene_camera_position);
    float3 reflected_direction = normalize(reflect(dir_to_pixel, scene_pixel_normal));
    float3 reflected_color = float3(0, 0, 0);
    float step_distance = 0;
    const float STEP_SIZE = 0.1;
    while (step_distance < 5) {
        step_distance += STEP_SIZE;
        float3 ray_position = scene_pixel_position + reflected_direction * step_distance;
        float4 ray_position_viewport_space = mul(camera_matrix, float4(ray_position, 1.0));
        ray_position_viewport_space /= ray_position_viewport_space.w;
        float2 ray_position_uv = ray_position_viewport_space.xy * 0.5 + 0.5;
        ray_position_uv.y = 1.0 - ray_position_uv.y;
        float3 ray_position_position_sample = positions_map.Sample(main_sampler, ray_position_uv).rgb;
        float4 ray_position_position_sample_viewport_space = mul(camera_matrix, float4(ray_position_position_sample, 1.0));
        ray_position_position_sample_viewport_space /= ray_position_position_sample_viewport_space.w;
        if (ray_position_viewport_space.z > ray_position_position_sample_viewport_space.z &&
            (length(ray_position - ray_position_position_sample) < STEP_SIZE) &&
            ray_position_viewport_space.x < 1 &&
            ray_position_viewport_space.x > -1 &&
            ray_position_viewport_space.y < 1 &&
            ray_position_viewport_space.y > -1) {

            float3 ray_position_screen_sample = scene_map.Sample(main_sampler, ray_position_uv).rgb;

            float3 offset_to_light = ray_position - scene_pixel_position;
            float distance_to_light = length(offset_to_light);
            // todo(josh): this attenuation is to make the edges of the screen reflection a little softer but it looks bad so figure out something better
            float attentuation_fade_term = 1.0 - (saturate(length(ray_position_viewport_space.xy)));
            // todo(josh): attenuate ray_position_screen_sample?
            // todo(josh): 0 or 1 for analytic parameter?
            float attentuation = saturate(1.0 / (distance_to_light * distance_to_light));
            reflected_color.rgb = attentuation_fade_term * calculate_light(base_color, metal_roughness_sample.x, metal_roughness_sample.y, scene_pixel_normal, -dir_to_pixel, reflected_direction, ray_position_screen_sample * attentuation, 1);
            break;
        }
    }
    // float4 main_scene_sample = scene_map.Sample(main_sampler, screen_uv);
    // main_scene_sample.rgb += reflected_color;
    return float4(reflected_color, 1.0);
    // float4 scene_color = scene_map.Sample(main_sampler, input.texcoord.xy);
    // return float4(scene_color.r, 0, 0, 1);
}
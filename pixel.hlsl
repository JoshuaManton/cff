SamplerState main_sampler;

Texture2D albedo : register(t0);

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
}

float4 main(PS_INPUT input) : SV_Target {
    float4 color = albedo.Sample(main_sampler, input.texcoord.xy);
    color.rgb *= 1.0f-smoothstep(0.0f, 50.0f, length(camera_position - input.world_position));
    return color;
}
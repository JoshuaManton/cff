SamplerState main_sampler;
Texture2D albedo : register(t0);

struct PS_INPUT {
  float4 position : SV_POSITION;
  float3 texcoord : TEXCOORD;
  float4 color    : COLOR;
};

float4 main(PS_INPUT input) : SV_Target {
  return albedo.Sample(main_sampler, input.texcoord.xy);
}
struct VS_INPUT {
  float3 position : SV_POSITION;
  float3 texcoord : TEXCOORD;
  float4 color    : COLOR;
};

struct PS_INPUT {
  float4 position : SV_POSITION;
  float3 texcoord : TEXCOORD;
  float4 color    : COLOR;
};

PS_INPUT main(VS_INPUT input) {
  PS_INPUT v;
  v.position = float4(input.position, 1.0);
  v.texcoord = input.texcoord;
  v.color = input.color;
  return v;
}
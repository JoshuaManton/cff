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

cbuffer CBUFFER_PASS : register(b0) {
    row_major matrix view_matrix;
    row_major matrix projection_matrix;
}

cbuffer CBUFFER_SPECIFIC : register(b1) {
    row_major matrix model_matrix;
};

PS_INPUT main(VS_INPUT input) {
    matrix mvp = mul(model_matrix, mul(view_matrix, projection_matrix));
    PS_INPUT v;
    v.position = mul(float4(input.position, 1.0), mvp);
    v.texcoord = input.texcoord;
    v.color = input.color;
    return v;
}
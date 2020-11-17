cbuffer CBUFFER_VP : register(b0) {
    matrix view_matrix;
    matrix projection_matrix;
};

RWTexture3D<float4> write_texture : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 thread_id : SV_DispatchThreadID, uint3 coords : SV_GroupThreadID) {
    matrix vp = mul(view_matrix, projection_matrix);
    float3 pos = float3((float)thread_id.x, (float)thread_id.y, (float)thread_id.z);
    pos /= 128;
    pos.y = 1 - pos.y;
    pos *= 2;
    pos -= float3(1, 1, 0);
    pos.z /= 2;
    float4 position = mul(view_matrix, float4(pos, 1.0));
    // float4 position = mul(vp, float4(pos, 1.0));
    // position /= position.w;
    float fog_density = exp(-position.y);
    write_texture[thread_id.xyz] = float4(fog_density, fog_density, fog_density, 1);
}
RWTexture3D<float4> write_texture : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 thread_id : SV_DispatchThreadID) {
    write_texture[thread_id.xyz] = float4(1, ((float)thread_id.z)/256, 1, 1);
}
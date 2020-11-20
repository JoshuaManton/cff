#define PI 3.14159265359

float3 fresnel_schlick(float cosTheta, float3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float distribution_ggx(float3 N, float3 H, float roughness) {
    float a      = roughness*roughness;
    float NdotH  = max(dot(N, H), 0.001);
    float NdotH2 = NdotH*NdotH;

    float num   = a;
    float denom = (NdotH2 * (a - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float geometry_schlick_ggx(float NdotV, float roughness, int analytic) {
    // node(josh): (roughness + 1) should only be used for analytic light sources, not IBL
    // "if applied to image-based lighting, the results at glancing angles will be much too dark"
    // page 3
    // https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf

    float k;
    if (analytic == 1) {
        k = pow((roughness + 1.0), 2.0) * 0.125;
    }
    else {
        k = pow(roughness, 2.0) * 0.5;
    }

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}
float geometry_smith(float3 N, float3 V, float3 L, float roughness, int analytic) {
    float NdotV = max(dot(N, V), 0.001);
    float NdotL = max(dot(N, L), 0.001);
    float ggx2  = geometry_schlick_ggx(NdotV, roughness, analytic);
    float ggx1  = geometry_schlick_ggx(NdotL, roughness, analytic);

    return ggx1 * ggx2;
}

float3 calculate_light(float3 albedo, float metallic, float roughness, float3 N, float3 V, float3 L, float3 incoming_radiance, int analytic) {
    float3 H = normalize(V + L);

    // todo(josh): no need to do this for each light, should be constant for a given draw call
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metallic);

    // cook-torrance brdf
    float  NDF = distribution_ggx(N, H, roughness);
    float  G   = geometry_smith(N, V, L, roughness, analytic);
    float3 F   = fresnel_schlick(saturate(dot(H, V)), F0);

    float3 kS = F;
    float3 kD = float3(1.0, 1.0, 1.0) - kS;
    kD *= 1.0 - metallic;

    float3 numerator  = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.001) * max(dot(N, L), 0.001);
    float3 specular   = numerator / max(denominator, 0.001);

    float NdotL = max(dot(N, L), 0.001);
    return (kD * albedo / PI + specular) * incoming_radiance * NdotL;
}
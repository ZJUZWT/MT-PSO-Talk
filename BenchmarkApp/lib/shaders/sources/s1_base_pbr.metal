#include <metal_stdlib>
using namespace metal;

struct SceneUBO {
    float4x4 model;
    float4x4 view;
    float4x4 projection;
    float3   cameraPos;
    float    _pad0;
};

struct LightUBO {
    float3 lightDir;
    float  lightIntensity;
    float3 lightColor;
    float  ambientIntensity;
    float  alphaClip;
    float  _pad1;
    float  _pad2;
    float  _pad3;
};

struct VertexIn {
    float3 position  [[attribute(0)]];
    float3 normal    [[attribute(1)]];
    float4 tangent   [[attribute(2)]];
    float2 texCoord  [[attribute(3)]];
};

struct VertexOut {
    float4 position   [[position]];
    float3 worldPos;
    float2 texCoord;
    float3 normal;
    float3 tangent;
    float3 bitangent;
    float3 viewDir;
};

vertex VertexOut s1_base_pbr_vertex(
    VertexIn           in       [[stage_in]],
    constant SceneUBO& scene    [[buffer(0)]])
{
    VertexOut out;
    float4 worldPos = scene.model * float4(in.position, 1.0);
    out.worldPos    = worldPos.xyz;

    float3x3 normalMat = float3x3(scene.model[0].xyz,
                                   scene.model[1].xyz,
                                   scene.model[2].xyz);
    out.normal    = normalize(normalMat * in.normal);
    out.tangent   = normalize(normalMat * in.tangent.xyz);
    out.bitangent = cross(out.normal, out.tangent) * in.tangent.w;

    out.texCoord = in.texCoord;
    out.viewDir  = normalize(scene.cameraPos - worldPos.xyz);
    out.position = scene.projection * scene.view * worldPos;
    return out;
}

constant float PI = 3.14159265359;

static float distributionGGX(float3 N, float3 H, float roughness) {
    float a  = roughness * roughness;
    float a2 = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom  = NdotH2 * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;
    return a2 / max(denom, 0.0001);
}

static float geometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

static float geometrySmith(float3 N, float3 V, float3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return geometrySchlickGGX(NdotV, roughness) *
           geometrySchlickGGX(NdotL, roughness);
}

static float3 fresnelSchlick(float cosTheta, float3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

fragment float4 s1_base_pbr_fragment(
    VertexOut          in          [[stage_in]],
    constant LightUBO& light       [[buffer(1)]],
    texture2d<float>   texAlbedo   [[texture(0)]],
    texture2d<float>   texNormal   [[texture(1)]],
    texture2d<float>   texRough    [[texture(2)]],
    texture2d<float>   texMetal    [[texture(3)]],
    texture2d<float>   texAO       [[texture(4)]],
    texture2d<float>   texEmissive [[texture(5)]],
    sampler            samp        [[sampler(0)]])
{
    float4 albedoSample = texAlbedo.sample(samp, in.texCoord);
    float3 albedo       = pow(albedoSample.rgb, float3(2.2));
    float  alpha        = albedoSample.a;

    float3 normalMap = texNormal.sample(samp, in.texCoord).rgb * 2.0 - 1.0;
    float  roughness = texRough.sample(samp, in.texCoord).r;
    float  metallic  = texMetal.sample(samp, in.texCoord).r;
    float  ao        = texAO.sample(samp, in.texCoord).r;
    float3 emissive  = texEmissive.sample(samp, in.texCoord).rgb;

    if (alpha < light.alphaClip) {
        discard_fragment();
    } else {
        alpha = 1.0;
    }

    float3x3 TBN = float3x3(normalize(in.tangent),
                              normalize(in.bitangent),
                              normalize(in.normal));
    float3 N = normalize(TBN * normalMap);
    float3 V = normalize(in.viewDir);

    float3 F0 = mix(float3(0.04), albedo, metallic);
    float3 L  = normalize(-light.lightDir);
    float3 H  = normalize(V + L);

    float  NDF = distributionGGX(N, H, roughness);
    float  G   = geometrySmith(N, V, L, roughness);
    float3 F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

    float3 numerator   = NDF * G * F;
    float  denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    float3 specular    = numerator / denominator;

    float3 kS = F;
    float3 kD = (float3(1.0) - kS) * (1.0 - metallic);

    float  NdotL    = max(dot(N, L), 0.0);
    float3 radiance = light.lightColor * light.lightIntensity;
    float3 Lo       = (kD * albedo / PI + specular) * radiance * NdotL;

    float3 ambient = float3(light.ambientIntensity) * albedo * ao;
    float3 color   = ambient + Lo + emissive;

    color = color / (color + float3(1.0));
    color = pow(color, float3(1.0 / 2.2));

    return float4(color, alpha);
}

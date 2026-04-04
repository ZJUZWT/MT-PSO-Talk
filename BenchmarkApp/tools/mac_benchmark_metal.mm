// Mac Benchmark — Metal PSO compilation timing for all 4 shader tiers
// Compiled as ObjC++ (.mm), links Metal + Foundation frameworks.
// MSL sources embedded directly (from lib/shaders/shader_loader.cpp).

#import <Metal/Metal.h>
#import <Foundation/Foundation.h>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

// Shared struct with mac_benchmark.cpp
struct MetalTierResult {
    std::string tier;
    int64_t compile_us;     // newLibraryWithSource
    int64_t pipeline_us;    // newRenderPipelineState
    int64_t total_us;
    bool ok;
};

static int64_t metal_now_us() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

// ═══════════════════════════════════════════════════════════
// Embedded MSL sources for all 4 tiers
// ═══════════════════════════════════════════════════════════

static const char* k_s1_msl = R"MSL(
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
)MSL";

static const char* k_s2_msl = R"MSL(
#include <metal_stdlib>
using namespace metal;

struct SceneUniforms {
    float4x4 model;
    float4x4 view;
    float4x4 projection;
    float4   cameraPos;
    float4   lightDir;
    float4   lightColor;
    float4   ambientColor;
    float4   layerParams;
};

struct VertexIn {
    float3 position   [[attribute(0)]];
    float3 normal     [[attribute(1)]];
    float4 tangent    [[attribute(2)]];
    float2 texCoord   [[attribute(3)]];
    float4 color      [[attribute(4)]];
};

struct VertexOut {
    float4 position      [[position]];
    float3 worldPos;
    float2 texCoord;
    float3 normal;
    float3 tangent;
    float3 bitangent;
    float4 vertexColor;
    float3 localPos;
};

vertex VertexOut s2_vertex(
    VertexIn in [[stage_in]],
    constant SceneUniforms& scene [[buffer(0)]]
) {
    VertexOut out;
    float4 worldPos = scene.model * float4(in.position, 1.0);
    out.worldPos  = worldPos.xyz;
    out.localPos  = in.position;

    float3x3 normalMatrix = float3x3(
        scene.model[0].xyz, scene.model[1].xyz, scene.model[2].xyz
    );
    out.normal    = normalize(normalMatrix * in.normal);
    out.tangent   = normalize(normalMatrix * in.tangent.xyz);
    out.bitangent = cross(out.normal, out.tangent) * in.tangent.w;

    out.texCoord    = in.texCoord;
    out.vertexColor = in.color;
    out.position    = scene.projection * scene.view * worldPos;
    return out;
}

constexpr float PI = 3.14159265359;

static float distributionGGX(float NdotH, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float d = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}

static float geometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

static float geometrySmith(float NdotV, float NdotL, float roughness) {
    return geometrySchlickGGX(NdotV, roughness) * geometrySchlickGGX(NdotL, roughness);
}

static float3 fresnelSchlick(float cosTheta, float3 F0) {
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

struct MaterialSample {
    float3 albedo;
    float3 normal;
    float  roughness;
    float  metallic;
    float  ao;
    float3 emissive;
};

static MaterialSample sampleLayerUV(
    texture2d<float> tAlb, texture2d<float> tNrm, texture2d<float> tRgh,
    texture2d<float> tMet, texture2d<float> tAo,  texture2d<float> tEms,
    sampler s, float2 uv
) {
    MaterialSample m;
    m.albedo    = pow(tAlb.sample(s, uv).rgb, float3(2.2));
    m.normal    = tNrm.sample(s, uv).rgb * 2.0 - 1.0;
    m.roughness = tRgh.sample(s, uv).r;
    m.metallic  = tMet.sample(s, uv).r;
    m.ao        = tAo.sample(s, uv).r;
    m.emissive  = tEms.sample(s, uv).rgb;
    return m;
}

constant bool LAYER_BLEND_HEIGHT       [[function_constant(0)]];
constant bool LAYER_BLEND_VERTEX_COLOR [[function_constant(1)]];
constant bool TRIPLANAR_PROJECTION     [[function_constant(2)]];
constant bool DETAIL_UV_SCALE          [[function_constant(3)]];

fragment float4 s2_fragment(
    VertexOut in [[stage_in]],
    constant SceneUniforms& scene [[buffer(0)]],
    texture2d<float> texAlbedo0    [[texture(0)]],
    texture2d<float> texNormal0    [[texture(1)]],
    texture2d<float> texRoughness0 [[texture(2)]],
    texture2d<float> texMetallic0  [[texture(3)]],
    texture2d<float> texAO0        [[texture(4)]],
    texture2d<float> texEmissive0  [[texture(5)]],
    texture2d<float> texAlbedo1    [[texture(6)]],
    texture2d<float> texNormal1    [[texture(7)]],
    texture2d<float> texRoughness1 [[texture(8)]],
    texture2d<float> texMetallic1  [[texture(9)]],
    texture2d<float> texAO1        [[texture(10)]],
    texture2d<float> texEmissive1  [[texture(11)]],
    sampler samp [[sampler(0)]]
) {
    float2 uv0 = in.texCoord;
    float2 uv1 = DETAIL_UV_SCALE ? in.texCoord * 4.0 : in.texCoord;

    MaterialSample layer0 = sampleLayerUV(
        texAlbedo0, texNormal0, texRoughness0,
        texMetallic0, texAO0, texEmissive0, samp, uv0);

    MaterialSample layer1 = sampleLayerUV(
        texAlbedo1, texNormal1, texRoughness1,
        texMetallic1, texAO1, texEmissive1, samp, uv1);

    float blendFactor = 0.5;
    if (LAYER_BLEND_HEIGHT) {
        float sharpness = scene.layerParams.x;
        float offset    = scene.layerParams.y;
        blendFactor = saturate((layer1.ao - layer0.ao + offset) * sharpness);
    }
    if (LAYER_BLEND_VERTEX_COLOR) {
        blendFactor *= in.vertexColor.r;
    }

    float3 albedo    = mix(layer0.albedo,    layer1.albedo,    blendFactor);
    float3 nrmTS     = mix(layer0.normal,    layer1.normal,    blendFactor);
    float  roughness = mix(layer0.roughness, layer1.roughness, blendFactor);
    float  metallic  = mix(layer0.metallic,  layer1.metallic,  blendFactor);
    float  ao        = mix(layer0.ao,        layer1.ao,        blendFactor);
    float3 emissive  = mix(layer0.emissive,  layer1.emissive,  blendFactor);

    float3 N = normalize(in.normal);
    float3 T = normalize(in.tangent);
    float3 B = normalize(in.bitangent);
    float3x3 TBN = float3x3(T, B, N);
    N = normalize(TBN * normalize(nrmTS));

    float3 V = normalize(scene.cameraPos.xyz - in.worldPos);
    float3 L = normalize(-scene.lightDir.xyz);
    float3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.001);
    float NdotH = max(dot(N, H), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    float3 F0 = mix(float3(0.04), albedo, metallic);
    float  D  = distributionGGX(NdotH, roughness);
    float  G  = geometrySmith(NdotV, NdotL, roughness);
    float3 F  = fresnelSchlick(HdotV, F0);

    float3 spec = (D * G * F) / (4.0 * NdotV * NdotL + 0.0001);
    float3 kD = (1.0 - F) * (1.0 - metallic);

    float3 radiance = scene.lightColor.rgb * scene.lightDir.w;
    float3 Lo = (kD * albedo / PI + spec) * radiance * NdotL;
    float3 ambient = scene.ambientColor.rgb * albedo * ao;
    float3 color = ambient + Lo + emissive;

    color = color / (color + 1.0);
    color = pow(color, float3(1.0 / 2.2));

    return float4(color, 1.0);
}
)MSL";

static const char* k_s3_msl = R"MSL(
#include <metal_stdlib>
using namespace metal;

struct PointLight {
    float3 position;
    float  radius;
    float3 color;
    float  intensity;
};

struct SceneUBO {
    float4x4 model;
    float4x4 view;
    float4x4 projection;
    float3   cameraPos;
    float    time;
};

struct LightUBO {
    float3     dirLightDir;
    float      dirLightIntensity;
    float3     dirLightColor;
    float      ambientIntensity;
    PointLight pointLights[4];
    float      clearCoatRoughness;
    float      clearCoatStrength;
    float      subsurfaceWidth;
    float      subsurfacePower;
    float3     subsurfaceColor;
    float      anisotropyStrength;
    float3     anisotropyDirection;
    float      parallaxScale;
    float      parallaxMinLayers;
    float      parallaxMaxLayers;
    float      fogDensity;
    float      fogHeight;
    float3     fogColor;
    float      emissivePulseSpeed;
};

struct VertexIn {
    float3 position   [[attribute(0)]];
    float3 normal     [[attribute(1)]];
    float4 tangent    [[attribute(2)]];
    float2 texCoord   [[attribute(3)]];
    float2 texCoord2  [[attribute(4)]];
};

struct VertexOut {
    float4 position   [[position]];
    float3 worldPos;
    float2 texCoord;
    float2 texCoord2;
    float3 normal;
    float3 tangent;
    float3 bitangent;
    float3 viewDir;
    float3 viewDirTS;
};

vertex VertexOut s3_feature_heavy_vertex(
    VertexIn           in    [[stage_in]],
    constant SceneUBO& scene [[buffer(0)]])
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
    out.texCoord  = in.texCoord;
    out.texCoord2 = in.texCoord2;
    out.viewDir   = normalize(scene.cameraPos - worldPos.xyz);

    float3x3 TBN   = float3x3(out.tangent, out.bitangent, out.normal);
    out.viewDirTS   = normalize(transpose(TBN) * out.viewDir);
    out.position    = scene.projection * scene.view * worldPos;
    return out;
}

constant bool fcClearCoat       [[function_constant(0)]];
constant bool fcSubsurface      [[function_constant(1)]];
constant bool fcAnisotropy      [[function_constant(2)]];
constant bool fcParallax        [[function_constant(3)]];
constant bool fcDetailNormal    [[function_constant(4)]];
constant bool fcEmissivePulse   [[function_constant(5)]];
constant bool fcAOMultiBounce   [[function_constant(6)]];
constant bool fcHeightFog       [[function_constant(7)]];

constant float PI = 3.14159265359;

static float distributionGGX(float3 N, float3 H, float roughness) {
    float a = roughness * roughness; float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float d = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d + 0.0001);
}

static float distributionGGXAniso(float3 N, float3 H, float3 T, float3 B,
                                   float ax, float ay) {
    float TdotH = dot(T, H); float BdotH = dot(B, H);
    float NdotH = max(dot(N, H), 0.0);
    float d = (TdotH*TdotH)/(ax*ax) + (BdotH*BdotH)/(ay*ay) + NdotH*NdotH;
    return 1.0 / (PI * ax * ay * d * d + 0.0001);
}

static float geometrySmith(float3 N, float3 V, float3 L, float r) {
    float k = (r + 1.0) * (r + 1.0) / 8.0;
    float NdotV = max(dot(N, V), 0.0); float NdotL = max(dot(N, L), 0.0);
    return (NdotV / (NdotV * (1.0 - k) + k)) * (NdotL / (NdotL * (1.0 - k) + k));
}

static float3 fresnelSchlick(float cosTheta, float3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

static float3 subsurfaceScatter(float3 N, float3 L, float3 V,
                                 float thickness, float curvature,
                                 constant LightUBO& lights) {
    float wrap = (dot(N, L) + lights.subsurfaceWidth) /
                 ((1.0 + lights.subsurfaceWidth) * (1.0 + lights.subsurfaceWidth));
    wrap = max(wrap, 0.0);
    float3 scatter = lights.subsurfaceColor * wrap * (1.0 - thickness) * mix(0.5, 1.0, curvature);
    float3 backDir = normalize(L + N * 0.3);
    float backLit = pow(max(dot(V, -backDir), 0.0), lights.subsurfacePower) * (1.0 - thickness);
    return scatter + lights.subsurfaceColor * backLit * 0.5;
}

static float2 parallaxOcclusionMap(float2 uv, float3 viewDirTS,
                                    constant LightUBO& lights,
                                    texture2d<float> texHeight,
                                    sampler samp) {
    if (!fcParallax) return uv;
    float numLayers = mix(lights.parallaxMaxLayers, lights.parallaxMinLayers,
                          max(dot(float3(0,0,1), viewDirTS), 0.0));
    float layerDepth = 1.0 / numLayers;
    float currentDepth = 0.0;
    float2 deltaUV = viewDirTS.xy / viewDirTS.z * lights.parallaxScale / numLayers;
    float2 currentUV = uv;
    float heightVal = 1.0 - texHeight.sample(samp, currentUV).r;
    for (int i = 0; i < 32; i++) {
        if (currentDepth >= heightVal) break;
        currentUV -= deltaUV;
        heightVal = 1.0 - texHeight.sample(samp, currentUV).r;
        currentDepth += layerDepth;
    }
    float2 prevUV = currentUV + deltaUV;
    float afterH  = heightVal - currentDepth;
    float beforeH = (1.0 - texHeight.sample(samp, prevUV).r) - currentDepth + layerDepth;
    float w = afterH / (afterH - beforeH);
    return mix(currentUV, prevUV, w);
}

static float3 evaluateLight(float3 N, float3 V, float3 L, float3 radiance,
                             float3 albedo, float roughness, float metallic,
                             float3 F0, float3 T, float3 B,
                             float thickness, float curvature,
                             constant LightUBO& lights) {
    float3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    float NDF;
    if (fcAnisotropy) {
        float aspect = sqrt(1.0 - lights.anisotropyStrength * 0.9);
        NDF = distributionGGXAniso(N, H, T, B,
                                    max(roughness/aspect, 0.001),
                                    max(roughness*aspect, 0.001));
    } else {
        NDF = distributionGGX(N, H, roughness);
    }
    float  G = geometrySmith(N, V, L, roughness);
    float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    float3 spec = (NDF * G * F) / (4.0 * max(dot(N,V),0.0) * NdotL + 0.0001);
    float3 kD = (float3(1.0) - F) * (1.0 - metallic);
    float3 Lo = (kD * albedo / PI + spec) * radiance * NdotL;
    if (fcSubsurface && thickness < 0.99) {
        float3 sss = subsurfaceScatter(N, L, V, thickness, curvature, lights);
        Lo += sss * radiance * albedo;
    }
    return Lo;
}

fragment float4 s3_feature_heavy_fragment(
    VertexOut              in             [[stage_in]],
    constant LightUBO&     lights         [[buffer(1)]],
    constant SceneUBO&     scene          [[buffer(0)]],
    texture2d<float>       texAlbedo      [[texture(0)]],
    texture2d<float>       texNormal      [[texture(1)]],
    texture2d<float>       texRoughness   [[texture(2)]],
    texture2d<float>       texMetallic    [[texture(3)]],
    texture2d<float>       texAO          [[texture(4)]],
    texture2d<float>       texEmissive    [[texture(5)]],
    texture2d<float>       texHeight      [[texture(6)]],
    texture2d<float>       texCCNormal    [[texture(7)]],
    texture2d<float>       texCCRoughness [[texture(8)]],
    texture2d<float>       texSSSThick    [[texture(9)]],
    texture2d<float>       texCurvature   [[texture(10)]],
    texture2d<float>       texAnisoAngle  [[texture(11)]],
    texture2d<float>       texDetailNrm   [[texture(12)]],
    texture2d<float>       texDetailMask  [[texture(13)]],
    texture2d<float>       texBRDFLut     [[texture(14)]],
    texture2d<float>       texNoise       [[texture(15)]],
    sampler                samp           [[sampler(0)]])
{
    float3 V = normalize(in.viewDir);
    float2 uv  = parallaxOcclusionMap(in.texCoord, in.viewDirTS, lights, texHeight, samp);
    float2 uv2 = in.texCoord2;

    float4 albedoS = texAlbedo.sample(samp, uv);
    float3 albedo  = pow(albedoS.rgb, float3(2.2));
    float  alpha   = albedoS.a;
    float3 nrmMap  = texNormal.sample(samp, uv).rgb * 2.0 - 1.0;
    float  rough   = texRoughness.sample(samp, uv).r;
    float  metal   = texMetallic.sample(samp, uv).r;
    float  ao      = texAO.sample(samp, uv).r;
    float3 emissive= texEmissive.sample(samp, uv).rgb;

    if (fcDetailNormal) {
        float mask = texDetailMask.sample(samp, uv).r;
        float3 detN = texDetailNrm.sample(samp, uv2 * 4.0).rgb * 2.0 - 1.0;
        nrmMap = normalize(float3(nrmMap.xy + detN.xy * mask, nrmMap.z));
    }

    float3x3 TBN = float3x3(normalize(in.tangent), normalize(in.bitangent), normalize(in.normal));
    float3 N = normalize(TBN * nrmMap);

    float3 T = normalize(in.tangent);
    float3 B = normalize(in.bitangent);
    if (fcAnisotropy) {
        float angle = texAnisoAngle.sample(samp, uv).r * PI * 2.0;
        T = normalize(T * cos(angle) + B * sin(angle));
        B = cross(N, T);
    }

    float thickness = 1.0, curvature = 0.5;
    if (fcSubsurface) {
        thickness = texSSSThick.sample(samp, uv).r;
        curvature = texCurvature.sample(samp, uv).r;
    }

    float3 ccNormal = N; float ccRough = lights.clearCoatRoughness;
    if (fcClearCoat) {
        ccNormal = normalize(TBN * (texCCNormal.sample(samp, uv).rgb * 2.0 - 1.0));
        ccRough  = texCCRoughness.sample(samp, uv).r;
    }

    float3 F0 = mix(float3(0.04), albedo, metal);
    float3 Lo = float3(0.0);

    {
        float3 L = normalize(-lights.dirLightDir);
        float3 rad = lights.dirLightColor * lights.dirLightIntensity;
        Lo += evaluateLight(N, V, L, rad, albedo, rough, metal, F0, T, B, thickness, curvature, lights);
    }

    for (int i = 0; i < 4; i++) {
        float3 toL = lights.pointLights[i].position - in.worldPos;
        float dist = length(toL);
        if (dist < lights.pointLights[i].radius) {
            float3 L = toL / dist;
            float atten = 1.0 / (dist * dist + 1.0);
            float falloff = clamp(1.0 - dist / lights.pointLights[i].radius, 0.0, 1.0);
            falloff *= falloff;
            float3 rad = lights.pointLights[i].color * lights.pointLights[i].intensity * atten * falloff;
            Lo += evaluateLight(N, V, L, rad, albedo, rough, metal, F0, T, B, thickness, curvature, lights);
        }
    }

    if (fcClearCoat) {
        float3 ccF0 = float3(0.04);
        float3 L = normalize(-lights.dirLightDir);
        float3 H = normalize(V + L);
        float ccNDF = distributionGGX(ccNormal, H, ccRough);
        float ccG   = geometrySmith(ccNormal, V, L, ccRough);
        float3 ccF  = fresnelSchlick(max(dot(H, V), 0.0), ccF0);
        float3 ccSpec = (ccNDF * ccG * ccF) /
                        (4.0 * max(dot(ccNormal,V),0.0) * max(dot(ccNormal,L),0.0) + 0.0001);
        Lo += ccSpec * lights.dirLightColor * lights.dirLightIntensity *
              max(dot(ccNormal, L), 0.0) * lights.clearCoatStrength;
        Lo *= (1.0 - ccF * lights.clearCoatStrength);
    }

    if (fcAOMultiBounce) {
        float3 a2 = 2.0 * albedo * ao;
        float3 b2 = albedo + ao;
        Lo *= mix(float3(ao), a2 / (b2 + 0.0001), 1.0 - ao);
    } else {
        Lo *= ao;
    }

    float3 ambient = float3(lights.ambientIntensity) * albedo * ao;

    if (fcEmissivePulse) {
        float pulse = sin(scene.time * lights.emissivePulseSpeed) * 0.5 + 0.5;
        emissive *= mix(0.5, 1.5, pulse);
    }

    float3 color = ambient + Lo + emissive;

    if (fcHeightFog) {
        float fogAmt = 1.0 - clamp(exp(-lights.fogDensity * max(0.0, lights.fogHeight - in.worldPos.y)), 0.0, 1.0);
        if (fogAmt > 0.01) color = mix(color, lights.fogColor, fogAmt);
    }

    float a_c = 2.51, b_c = 0.03, c_c = 2.43, d_c = 0.59, e_c = 0.14;
    color = clamp((color*(a_c*color+b_c))/(color*(c_c*color+d_c)+e_c), 0.0, 1.0);
    color = pow(color, float3(1.0/2.2));

    return float4(color, alpha);
}
)MSL";

static const char* k_s4_msl = R"MSL(
#include <metal_stdlib>
using namespace metal;

struct SceneUniforms {
    float4x4 model;
    float4x4 view;
    float4x4 projection;
    float4   cameraPos;
    float4   lightDir;
    float4   lightColor;
    float4   ambientColor;
    float4   fogParams;
    float4   shadowMatrix0;
    float4   shadowMatrix1;
    float4   shadowMatrix2;
    float4   shadowMatrix3;
};

struct VertexIn {
    float3 position  [[attribute(0)]];
    float3 normal    [[attribute(1)]];
    float4 tangent   [[attribute(2)]];
    float2 texCoord  [[attribute(3)]];
};

struct VertexOut {
    float4 position     [[position]];
    float3 worldPos;
    float2 texCoord;
    float3 normal;
    float3 tangent;
    float3 bitangent;
    float4 shadowCoord;
    float  viewDist;
};

vertex VertexOut s4_vertex(
    VertexIn in [[stage_in]],
    constant SceneUniforms& scene [[buffer(0)]]
) {
    VertexOut out;
    float4 worldPos = scene.model * float4(in.position, 1.0);
    out.worldPos = worldPos.xyz;

    float3x3 normalMatrix = float3x3(
        scene.model[0].xyz, scene.model[1].xyz, scene.model[2].xyz
    );
    out.normal    = normalize(normalMatrix * in.normal);
    out.tangent   = normalize(normalMatrix * in.tangent.xyz);
    out.bitangent = cross(out.normal, out.tangent) * in.tangent.w;

    out.texCoord = in.texCoord;

    float4x4 shadowMat = float4x4(
        scene.shadowMatrix0, scene.shadowMatrix1,
        scene.shadowMatrix2, scene.shadowMatrix3
    );
    out.shadowCoord = shadowMat * worldPos;
    out.viewDist = length(scene.cameraPos.xyz - worldPos.xyz);
    out.position = scene.projection * scene.view * worldPos;
    return out;
}

constexpr float PI = 3.14159265359;

static float distributionGGX(float NdotH, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float d = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}

static float geometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

static float geometrySmith(float NdotV, float NdotL, float roughness) {
    return geometrySchlickGGX(NdotV, roughness) * geometrySchlickGGX(NdotL, roughness);
}

static float3 fresnelSchlick(float cosTheta, float3 F0) {
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

static float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness) {
    return F0 + (max(float3(1.0 - roughness), F0) - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

static float sampleShadow(float4 shadowCoord, texture2d<float> shadowMap, sampler s) {
    float3 proj = shadowCoord.xyz / shadowCoord.w;
    proj = proj * 0.5 + 0.5;
    if (proj.z > 1.0) return 1.0;

    float shadow = 0.0;
    float texelSize = 1.0 / 2048.0;
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float d = shadowMap.sample(s, proj.xy + float2(x, y) * texelSize).r;
            shadow += (proj.z - 0.005 > d) ? 0.0 : 1.0;
        }
    }
    return shadow / 9.0;
}

constant bool HAS_NORMAL_MAP [[function_constant(0)]];
constant bool HAS_EMISSIVE   [[function_constant(1)]];
constant bool HAS_AO         [[function_constant(2)]];
constant bool USE_IBL        [[function_constant(3)]];
constant bool USE_SHADOWS    [[function_constant(4)]];
constant bool USE_FOG        [[function_constant(5)]];

fragment float4 s4_fragment(
    VertexOut in [[stage_in]],
    constant SceneUniforms& scene [[buffer(0)]],
    texture2d<float> texAlbedo      [[texture(0)]],
    texture2d<float> texNormal      [[texture(1)]],
    texture2d<float> texRoughness   [[texture(2)]],
    texture2d<float> texMetallic    [[texture(3)]],
    texture2d<float> texAO          [[texture(4)]],
    texture2d<float> texEmissive    [[texture(5)]],
    texture2d<float> texShadowMap   [[texture(6)]],
    texture2d<float> texIrradiance  [[texture(7)]],
    texture2d<float> texPrefiltered [[texture(8)]],
    texture2d<float> texBRDFLut     [[texture(9)]],
    texture2d<float> texFogNoise    [[texture(10)]],
    sampler samp [[sampler(0)]]
) {
    float4 albedoSample = texAlbedo.sample(samp, in.texCoord);
    float  roughness    = texRoughness.sample(samp, in.texCoord).r;
    float  metallic     = texMetallic.sample(samp, in.texCoord).r;

    float3 albedo = pow(albedoSample.rgb, float3(2.2));

    float3 N = normalize(in.normal);
    if (HAS_NORMAL_MAP) {
        float3 T = normalize(in.tangent);
        float3 B = normalize(in.bitangent);
        float3x3 TBN = float3x3(T, B, N);
        float3 ns = texNormal.sample(samp, in.texCoord).rgb * 2.0 - 1.0;
        N = normalize(TBN * ns);
    }

    float ao = 1.0;
    if (HAS_AO) {
        ao = texAO.sample(samp, in.texCoord).r;
    }

    float3 emissive = float3(0.0);
    if (HAS_EMISSIVE) {
        emissive = texEmissive.sample(samp, in.texCoord).rgb;
    }

    float3 V = normalize(scene.cameraPos.xyz - in.worldPos);
    float3 L = normalize(-scene.lightDir.xyz);
    float3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.001);
    float NdotH = max(dot(N, H), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    float3 F0 = mix(float3(0.04), albedo, metallic);
    float  D  = distributionGGX(NdotH, roughness);
    float  G  = geometrySmith(NdotV, NdotL, roughness);
    float3 F  = fresnelSchlick(HdotV, F0);

    float3 spec = (D * G * F) / (4.0 * NdotV * NdotL + 0.0001);
    float3 kD = (1.0 - F) * (1.0 - metallic);

    float3 radiance = scene.lightColor.rgb * scene.lightDir.w;

    float shadow = 1.0;
    if (USE_SHADOWS) {
        shadow = sampleShadow(in.shadowCoord, texShadowMap, samp);
    }

    float3 Lo = (kD * albedo / PI + spec) * radiance * NdotL * shadow;

    float3 ambient;
    if (USE_IBL) {
        float3 R = reflect(-V, N);
        float2 irrUV = float2(atan2(N.z, N.x) / (2.0 * PI) + 0.5, acos(N.y) / PI);
        float3 irradiance = texIrradiance.sample(samp, irrUV).rgb;

        float2 prefUV = float2(atan2(R.z, R.x) / (2.0 * PI) + 0.5, acos(R.y) / PI);
        float3 prefColor = texPrefiltered.sample(samp, prefUV, level(roughness * 8.0)).rgb;
        float2 brdf = texBRDFLut.sample(samp, float2(NdotV, roughness)).rg;

        float3 Fibl = fresnelSchlickRoughness(NdotV, F0, roughness);
        float3 kD_ibl = (1.0 - Fibl) * (1.0 - metallic);

        ambient = (kD_ibl * irradiance * albedo + prefColor * (Fibl * brdf.x + brdf.y)) * ao;
    } else {
        ambient = scene.ambientColor.rgb * albedo * ao;
    }

    float3 color = ambient + Lo + emissive;

    color = color / (color + 1.0);
    color = pow(color, float3(1.0 / 2.2));

    if (USE_FOG) {
        float fogStart   = scene.fogParams.x;
        float fogEnd     = scene.fogParams.y;
        float fogDensity = scene.fogParams.z;
        float fogMode    = scene.fogParams.w;

        float fogFactor;
        if (fogMode < 0.5) {
            fogFactor = saturate((fogEnd - in.viewDist) / (fogEnd - fogStart));
        } else {
            fogFactor = exp(-fogDensity * in.viewDist);
        }

        float noise = texFogNoise.sample(samp, in.worldPos.xz * 0.01).r;
        fogFactor *= mix(0.8, 1.0, noise);

        float3 fogColor = scene.ambientColor.rgb * 0.8;
        color = mix(fogColor, color, fogFactor);
    }

    return float4(color, 1.0);
}
)MSL";

// ═══════════════════════════════════════════════════════════
// Tier info
// ═══════════════════════════════════════════════════════════

struct MetalTierInfo {
    const char* display_name;
    const char* msl_source;
    const char* vert_func;
    const char* frag_func;
};

static const MetalTierInfo kMetalTiers[] = {
    { "S1 BasePBR",           k_s1_msl, "s1_base_pbr_vertex",      "s1_base_pbr_fragment"      },
    { "S2 MaterialLayered",   k_s2_msl, "s2_vertex",               "s2_fragment"               },
    { "S3 FeatureHeavy",      k_s3_msl, "s3_feature_heavy_vertex", "s3_feature_heavy_fragment" },
    { "S4 PermutationStress", k_s4_msl, "s4_vertex",               "s4_fragment"               },
};

static MetalTierResult benchmark_metal_tier(id<MTLDevice> device, const MetalTierInfo& info) {
    MetalTierResult result;
    result.tier = info.display_name;
    result.compile_us = 0;
    result.pipeline_us = 0;
    result.total_us = 0;
    result.ok = false;

    NSString* mslSource = [NSString stringWithUTF8String:info.msl_source];

    int64_t t_total = metal_now_us();

    // Compile MSL -> MTLLibrary
    NSError* error = nil;
    int64_t t0 = metal_now_us();
    id<MTLLibrary> library = [device newLibraryWithSource:mslSource options:nil error:&error];
    result.compile_us = metal_now_us() - t0;

    if (!library) {
        if (error) {
            std::cerr << "  [Metal] Compile error (" << info.display_name << "): "
                      << [[error localizedDescription] UTF8String] << "\n";
        }
        return result;
    }

    // Get vertex + fragment functions
    NSString* vertName = [NSString stringWithUTF8String:info.vert_func];
    NSString* fragName = [NSString stringWithUTF8String:info.frag_func];
    id<MTLFunction> vertFunc = [library newFunctionWithName:vertName];
    id<MTLFunction> fragFunc = [library newFunctionWithName:fragName];

    if (!vertFunc || !fragFunc) {
        std::cerr << "  [Metal] Cannot find functions: " << info.vert_func
                  << " / " << info.frag_func << "\n";
        return result;
    }

    // Create render pipeline descriptor
    MTLRenderPipelineDescriptor* desc = [[MTLRenderPipelineDescriptor alloc] init];
    desc.vertexFunction = vertFunc;
    desc.fragmentFunction = fragFunc;
    desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

    // Create PSO
    error = nil;
    t0 = metal_now_us();
    id<MTLRenderPipelineState> pso = [device newRenderPipelineStateWithDescriptor:desc error:&error];
    result.pipeline_us = metal_now_us() - t0;

    result.total_us = metal_now_us() - t_total;

    if (!pso) {
        if (error) {
            std::cerr << "  [Metal] PSO error (" << info.display_name << "): "
                      << [[error localizedDescription] UTF8String] << "\n";
        }
        return result;
    }

    result.ok = true;
    return result;
}

std::vector<MetalTierResult> run_metal_all_tiers() {
    std::vector<MetalTierResult> results;

    @autoreleasepool {
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        if (!device) {
            std::cerr << "  [Metal] No Metal device available\n";
            return results;
        }

        std::cout << "  Metal device: " << [[device name] UTF8String] << "\n";

        for (const auto& info : kMetalTiers) {
            auto r = benchmark_metal_tier(device, info);
            results.push_back(r);
        }
    }

    return results;
}

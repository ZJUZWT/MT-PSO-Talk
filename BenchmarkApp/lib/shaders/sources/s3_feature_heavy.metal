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

// 8 function constants
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

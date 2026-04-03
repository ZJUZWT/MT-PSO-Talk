#include <metal_stdlib>
using namespace metal;

// ──────────── Types ────────────

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

// ──────────── Vertex ────────────

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

// ──────────── PBR Helpers ────────────

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

// ──────────── Function constants (6 toggles = 64 permutations) ────────────

constant bool HAS_NORMAL_MAP [[function_constant(0)]];
constant bool HAS_EMISSIVE   [[function_constant(1)]];
constant bool HAS_AO         [[function_constant(2)]];
constant bool USE_IBL        [[function_constant(3)]];
constant bool USE_SHADOWS    [[function_constant(4)]];
constant bool USE_FOG        [[function_constant(5)]];

// ──────────── Fragment ────────────

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
    // Always-sampled
    float4 albedoSample = texAlbedo.sample(samp, in.texCoord);
    float  roughness    = texRoughness.sample(samp, in.texCoord).r;
    float  metallic     = texMetallic.sample(samp, in.texCoord).r;

    float3 albedo = pow(albedoSample.rgb, float3(2.2));

    // Normal
    float3 N = normalize(in.normal);
    if (HAS_NORMAL_MAP) {
        float3 T = normalize(in.tangent);
        float3 B = normalize(in.bitangent);
        float3x3 TBN = float3x3(T, B, N);
        float3 ns = texNormal.sample(samp, in.texCoord).rgb * 2.0 - 1.0;
        N = normalize(TBN * ns);
    }

    // AO
    float ao = 1.0;
    if (HAS_AO) {
        ao = texAO.sample(samp, in.texCoord).r;
    }

    // Emissive
    float3 emissive = float3(0.0);
    if (HAS_EMISSIVE) {
        emissive = texEmissive.sample(samp, in.texCoord).rgb;
    }

    // Vectors
    float3 V = normalize(scene.cameraPos.xyz - in.worldPos);
    float3 L = normalize(-scene.lightDir.xyz);
    float3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.001);
    float NdotH = max(dot(N, H), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    // Cook-Torrance
    float3 F0 = mix(float3(0.04), albedo, metallic);
    float  D  = distributionGGX(NdotH, roughness);
    float  G  = geometrySmith(NdotV, NdotL, roughness);
    float3 F  = fresnelSchlick(HdotV, F0);

    float3 spec = (D * G * F) / (4.0 * NdotV * NdotL + 0.0001);
    float3 kD = (1.0 - F) * (1.0 - metallic);

    float3 radiance = scene.lightColor.rgb * scene.lightDir.w;

    // Shadow
    float shadow = 1.0;
    if (USE_SHADOWS) {
        shadow = sampleShadow(in.shadowCoord, texShadowMap, samp);
    }

    float3 Lo = (kD * albedo / PI + spec) * radiance * NdotL * shadow;

    // Ambient / IBL
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

    // Tone map
    color = color / (color + 1.0);
    color = pow(color, float3(1.0 / 2.2));

    // Fog
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

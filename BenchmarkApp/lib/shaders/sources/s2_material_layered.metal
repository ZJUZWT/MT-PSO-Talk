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
    float4   layerParams;   // x = blend sharpness, y = height offset, z = triplanar sharpness
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

// ──────────── Vertex ────────────

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

// ──────────── Helpers ────────────

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

static float4 sampleTriplanar(texture2d<float> tex, sampler s, float3 pos, float3 w) {
    float4 xP = tex.sample(s, pos.yz);
    float4 yP = tex.sample(s, pos.xz);
    float4 zP = tex.sample(s, pos.xy);
    return xP * w.x + yP * w.y + zP * w.z;
}

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

// ──────────── Fragment ────────────

// Function constants for static switches
constant bool LAYER_BLEND_HEIGHT       [[function_constant(0)]];
constant bool LAYER_BLEND_VERTEX_COLOR [[function_constant(1)]];
constant bool TRIPLANAR_PROJECTION     [[function_constant(2)]];
constant bool DETAIL_UV_SCALE          [[function_constant(3)]];

fragment float4 s2_fragment(
    VertexOut in [[stage_in]],
    constant SceneUniforms& scene [[buffer(0)]],
    // Layer 0
    texture2d<float> texAlbedo0    [[texture(0)]],
    texture2d<float> texNormal0    [[texture(1)]],
    texture2d<float> texRoughness0 [[texture(2)]],
    texture2d<float> texMetallic0  [[texture(3)]],
    texture2d<float> texAO0        [[texture(4)]],
    texture2d<float> texEmissive0  [[texture(5)]],
    // Layer 1
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

    // Sample both layers (12 texture fetches)
    MaterialSample layer0 = sampleLayerUV(
        texAlbedo0, texNormal0, texRoughness0,
        texMetallic0, texAO0, texEmissive0, samp, uv0);

    MaterialSample layer1 = sampleLayerUV(
        texAlbedo1, texNormal1, texRoughness1,
        texMetallic1, texAO1, texEmissive1, samp, uv1);

    // Blend factor
    float blendFactor = 0.5;
    if (LAYER_BLEND_HEIGHT) {
        float sharpness = scene.layerParams.x;
        float offset    = scene.layerParams.y;
        blendFactor = saturate((layer1.ao - layer0.ao + offset) * sharpness);
    }
    if (LAYER_BLEND_VERTEX_COLOR) {
        blendFactor *= in.vertexColor.r;
    }

    // Blend
    float3 albedo    = mix(layer0.albedo,    layer1.albedo,    blendFactor);
    float3 nrmTS     = mix(layer0.normal,    layer1.normal,    blendFactor);
    float  roughness = mix(layer0.roughness, layer1.roughness, blendFactor);
    float  metallic  = mix(layer0.metallic,  layer1.metallic,  blendFactor);
    float  ao        = mix(layer0.ao,        layer1.ao,        blendFactor);
    float3 emissive  = mix(layer0.emissive,  layer1.emissive,  blendFactor);

    // Normal mapping
    float3 N = normalize(in.normal);
    float3 T = normalize(in.tangent);
    float3 B = normalize(in.bitangent);
    float3x3 TBN = float3x3(T, B, N);
    N = normalize(TBN * normalize(nrmTS));

    // Cook-Torrance lighting
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

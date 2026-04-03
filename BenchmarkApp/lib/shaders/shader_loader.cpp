#include "shader_loader.h"

namespace benchmark {

static const char* k_s1_basepbr_vert = R"SHADER_S1_BASEPBR_VERT(
#version 450

layout(set = 0, binding = 0) uniform SceneUBO {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 cameraPos;
    float _pad0;
} scene;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;   // .w = bitangent sign
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragTangent;
layout(location = 4) out vec3 fragBitangent;
layout(location = 5) out vec3 fragViewDir;

void main() {
    vec4 worldPos = scene.model * vec4(inPosition, 1.0);
    fragWorldPos  = worldPos.xyz;

    mat3 normalMat = transpose(inverse(mat3(scene.model)));
    fragNormal     = normalize(normalMat * inNormal);
    fragTangent    = normalize(normalMat * inTangent.xyz);
    fragBitangent  = cross(fragNormal, fragTangent) * inTangent.w;

    fragTexCoord = inTexCoord;
    fragViewDir  = normalize(scene.cameraPos - worldPos.xyz);

    gl_Position = scene.projection * scene.view * worldPos;
}
)SHADER_S1_BASEPBR_VERT";

static const char* k_s1_basepbr_frag = R"SHADER_S1_BASEPBR_FRAG(
#version 450

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragTangent;
layout(location = 4) in vec3 fragBitangent;
layout(location = 5) in vec3 fragViewDir;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform SceneUBO {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 cameraPos;
    float _pad0;
} scene;

layout(set = 0, binding = 1) uniform LightUBO {
    vec3 lightDir;
    float lightIntensity;
    vec3 lightColor;
    float ambientIntensity;
    float alphaClip;
    float _pad1;
    float _pad2;
    float _pad3;
} light;

layout(set = 1, binding = 0) uniform sampler2D texAlbedo;
layout(set = 1, binding = 1) uniform sampler2D texNormal;
layout(set = 1, binding = 2) uniform sampler2D texRoughness;
layout(set = 1, binding = 3) uniform sampler2D texMetallic;
layout(set = 1, binding = 4) uniform sampler2D texAO;
layout(set = 1, binding = 5) uniform sampler2D texEmissive;

const float PI = 3.14159265359;

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a  = roughness * roughness;
    float a2 = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;
    return a2 / max(denom, 0.0001);
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return geometrySchlickGGX(NdotV, roughness) *
           geometrySchlickGGX(NdotL, roughness);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    // Sample all 6 textures
    vec4  albedoSample = texture(texAlbedo, fragTexCoord);
    vec3  albedo       = pow(albedoSample.rgb, vec3(2.2));
    float alpha        = albedoSample.a;

    vec3  normalMap = texture(texNormal, fragTexCoord).rgb * 2.0 - 1.0;
    float roughness = texture(texRoughness, fragTexCoord).r;
    float metallic  = texture(texMetallic, fragTexCoord).r;
    float ao        = texture(texAO, fragTexCoord).r;
    vec3  emissive  = texture(texEmissive, fragTexCoord).rgb;

    // Alpha test (single branch)
    if (alpha < light.alphaClip) {
        discard;
    } else {
        alpha = 1.0;
    }

    // TBN normal
    mat3 TBN = mat3(normalize(fragTangent),
                     normalize(fragBitangent),
                     normalize(fragNormal));
    vec3 N = normalize(TBN * normalMap);
    vec3 V = normalize(fragViewDir);

    // Cook-Torrance BRDF
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 L = normalize(-light.lightDir);
    vec3 H = normalize(V + L);

    float NDF = distributionGGX(N, H, roughness);
    float G   = geometrySmith(N, V, L, roughness);
    vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3  numerator   = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3  specular    = numerator / denominator;

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

    float NdotL = max(dot(N, L), 0.0);
    vec3 radiance = light.lightColor * light.lightIntensity;
    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;

    // Ambient + AO + Emissive
    vec3 ambient = vec3(light.ambientIntensity) * albedo * ao;
    vec3 color   = ambient + Lo + emissive;

    // Tone-map (Reinhard) and gamma
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, alpha);
}
)SHADER_S1_BASEPBR_FRAG";

static const char* k_s1_basepbr_metal = R"SHADER_S1_BASEPBR_METAL(
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
)SHADER_S1_BASEPBR_METAL";

static const char* k_s2_materiallayered_vert = R"SHADER_S2_MATERIALLAYERED_VERT(
#version 450

layout(set = 0, binding = 0) uniform SceneUniforms {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 cameraPos;
    vec4 lightDir;
    vec4 lightColor;
    vec4 ambientColor;
    vec4 layerParams;    // x = blend sharpness, y = height offset, z = triplanar sharpness
} scene;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec4 inColor;   // vertex color – R channel used for layer blend mask

layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragTangent;
layout(location = 4) out vec3 fragBitangent;
layout(location = 5) out vec4 fragVertexColor;
layout(location = 6) out vec3 fragLocalPos;  // for triplanar

void main() {
    vec4 worldPos = scene.model * vec4(inPosition, 1.0);
    fragWorldPos  = worldPos.xyz;
    fragLocalPos  = inPosition;

    mat3 normalMatrix = transpose(inverse(mat3(scene.model)));
    fragNormal    = normalize(normalMatrix * inNormal);
    fragTangent   = normalize(normalMatrix * inTangent.xyz);
    fragBitangent = cross(fragNormal, fragTangent) * inTangent.w;

    fragTexCoord    = inTexCoord;
    fragVertexColor = inColor;

    gl_Position = scene.projection * scene.view * worldPos;
}
)SHADER_S2_MATERIALLAYERED_VERT";

static const char* k_s2_materiallayered_frag = R"SHADER_S2_MATERIALLAYERED_FRAG(
#version 450

// 4 static switches
// #define LAYER_BLEND_HEIGHT
// #define LAYER_BLEND_SLOPE
// #define USE_TRIPLANAR
// #define DETAIL_UV_SCALE

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragTangent;
layout(location = 4) in vec3 fragBitangent;
layout(location = 5) in vec3 fragViewDir;
layout(location = 6) in vec3 fragObjectPos;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform SceneUBO {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 cameraPos;
    float _pad0;
} scene;

layout(set = 0, binding = 1) uniform MaterialUBO {
    vec3  lightDir;
    float lightIntensity;
    vec3  lightColor;
    float ambientIntensity;
    float blendSharpness;
    float triplanarSharpness;
    float detailUVScale;
    float heightOffset;
} mat_ubo;

// Layer 0 textures (6)
layout(set = 1, binding = 0)  uniform sampler2D tex0Albedo;
layout(set = 1, binding = 1)  uniform sampler2D tex0Normal;
layout(set = 1, binding = 2)  uniform sampler2D tex0Roughness;
layout(set = 1, binding = 3)  uniform sampler2D tex0Metallic;
layout(set = 1, binding = 4)  uniform sampler2D tex0AO;
layout(set = 1, binding = 5)  uniform sampler2D tex0Height;

// Layer 1 textures (6)
layout(set = 1, binding = 6)  uniform sampler2D tex1Albedo;
layout(set = 1, binding = 7)  uniform sampler2D tex1Normal;
layout(set = 1, binding = 8)  uniform sampler2D tex1Roughness;
layout(set = 1, binding = 9)  uniform sampler2D tex1Metallic;
layout(set = 1, binding = 10) uniform sampler2D tex1AO;
layout(set = 1, binding = 11) uniform sampler2D tex1Height;

const float PI = 3.14159265359;

float distributionGGX(vec3 N, vec3 H, float r) {
    float a = r * r; float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float d = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d + 0.0001);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float r) {
    float k = (r + 1.0) * (r + 1.0) / 8.0;
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float gv = NdotV / (NdotV * (1.0 - k) + k);
    float gl = NdotL / (NdotL * (1.0 - k) + k);
    return gv * gl;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec4 sampleTriplanar(sampler2D tex, vec3 pos, vec3 blendWeights, float scale) {
    vec4 xProj = texture(tex, pos.yz * scale);
    vec4 yProj = texture(tex, pos.xz * scale);
    vec4 zProj = texture(tex, pos.xy * scale);
    return xProj * blendWeights.x + yProj * blendWeights.y + zProj * blendWeights.z;
}

struct MaterialSample {
    vec3  albedo;
    vec3  normal;
    float roughness;
    float metallic;
    float ao;
    float height;
};

MaterialSample sampleLayer(sampler2D tAlb, sampler2D tNrm, sampler2D tRgh,
                           sampler2D tMet, sampler2D tAO,  sampler2D tHgt,
                           vec2 uv)
{
    MaterialSample s;
    s.albedo    = pow(texture(tAlb, uv).rgb, vec3(2.2));
    s.normal    = texture(tNrm, uv).rgb * 2.0 - 1.0;
    s.roughness = texture(tRgh, uv).r;
    s.metallic  = texture(tMet, uv).r;
    s.ao        = texture(tAO, uv).r;
    s.height    = texture(tHgt, uv).r;
    return s;
}

void main() {
    vec2 uv0 = fragTexCoord;

#ifdef DETAIL_UV_SCALE
    vec2 uv1 = fragTexCoord * mat_ubo.detailUVScale;
#else
    vec2 uv1 = fragTexCoord;
#endif

    vec3 triWeights = vec3(0.0);
#ifdef USE_TRIPLANAR
    vec3 blendRaw = abs(normalize(fragNormal));
    blendRaw = pow(blendRaw, vec3(mat_ubo.triplanarSharpness));
    triWeights = blendRaw / (blendRaw.x + blendRaw.y + blendRaw.z);
#endif

    MaterialSample layer0, layer1;

#ifdef USE_TRIPLANAR
    float triScale = 1.0;
    layer0.albedo    = pow(sampleTriplanar(tex0Albedo, fragObjectPos, triWeights, triScale).rgb, vec3(2.2));
    layer0.normal    = sampleTriplanar(tex0Normal, fragObjectPos, triWeights, triScale).rgb * 2.0 - 1.0;
    layer0.roughness = sampleTriplanar(tex0Roughness, fragObjectPos, triWeights, triScale).r;
    layer0.metallic  = sampleTriplanar(tex0Metallic, fragObjectPos, triWeights, triScale).r;
    layer0.ao        = sampleTriplanar(tex0AO, fragObjectPos, triWeights, triScale).r;
    layer0.height    = sampleTriplanar(tex0Height, fragObjectPos, triWeights, triScale).r;

    layer1.albedo    = pow(sampleTriplanar(tex1Albedo, fragObjectPos, triWeights, triScale).rgb, vec3(2.2));
    layer1.normal    = sampleTriplanar(tex1Normal, fragObjectPos, triWeights, triScale).rgb * 2.0 - 1.0;
    layer1.roughness = sampleTriplanar(tex1Roughness, fragObjectPos, triWeights, triScale).r;
    layer1.metallic  = sampleTriplanar(tex1Metallic, fragObjectPos, triWeights, triScale).r;
    layer1.ao        = sampleTriplanar(tex1AO, fragObjectPos, triWeights, triScale).r;
    layer1.height    = sampleTriplanar(tex1Height, fragObjectPos, triWeights, triScale).r;
#else
    layer0 = sampleLayer(tex0Albedo, tex0Normal, tex0Roughness, tex0Metallic, tex0AO, tex0Height, uv0);
    layer1 = sampleLayer(tex1Albedo, tex1Normal, tex1Roughness, tex1Metallic, tex1AO, tex1Height, uv1);
#endif

    float blendFactor = 0.5;

#ifdef LAYER_BLEND_HEIGHT
    float h0 = layer0.height + mat_ubo.heightOffset;
    float h1 = layer1.height;
    float hDiff = h0 - h1;
    blendFactor = clamp(hDiff * mat_ubo.blendSharpness + 0.5, 0.0, 1.0);
#endif

#ifdef LAYER_BLEND_SLOPE
    float slope = 1.0 - max(dot(normalize(fragNormal), vec3(0.0, 1.0, 0.0)), 0.0);
    blendFactor = mix(blendFactor, slope, 0.7);
#endif

    vec3  albedo    = mix(layer0.albedo,    layer1.albedo,    blendFactor);
    vec3  normalTS  = mix(layer0.normal,    layer1.normal,    blendFactor);
    float roughness = mix(layer0.roughness, layer1.roughness, blendFactor);
    float metallic  = mix(layer0.metallic,  layer1.metallic,  blendFactor);
    float ao        = mix(layer0.ao,        layer1.ao,        blendFactor);

    mat3 TBN = mat3(normalize(fragTangent),
                     normalize(fragBitangent),
                     normalize(fragNormal));
    vec3 N = normalize(TBN * normalTS);
    vec3 V = normalize(fragViewDir);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 L  = normalize(-mat_ubo.lightDir);
    vec3 H  = normalize(V + L);

    float NDF = distributionGGX(N, H, roughness);
    float G   = geometrySmith(N, V, L, roughness);
    vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3  spec = (NDF * G * F) /
                 (4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001);

    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    float NdotL = max(dot(N, L), 0.0);
    vec3 radiance = mat_ubo.lightColor * mat_ubo.lightIntensity;
    vec3 Lo = (kD * albedo / PI + spec) * radiance * NdotL;

    vec3 ambient = vec3(mat_ubo.ambientIntensity) * albedo * ao;
    vec3 color   = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, 1.0);
}
)SHADER_S2_MATERIALLAYERED_FRAG";

static const char* k_s2_materiallayered_metal = R"SHADER_S2_MATERIALLAYERED_METAL(
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
)SHADER_S2_MATERIALLAYERED_METAL";

static const char* k_s3_featureheavy_vert = R"SHADER_S3_FEATUREHEAVY_VERT(
#version 450

layout(set = 0, binding = 0) uniform SceneUBO {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 cameraPos;
    float _pad0;
} scene;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec2 inTexCoord2;

layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec2 fragTexCoord2;
layout(location = 3) out vec3 fragNormal;
layout(location = 4) out vec3 fragTangent;
layout(location = 5) out vec3 fragBitangent;
layout(location = 6) out vec3 fragViewDir;
layout(location = 7) out vec3 fragViewDirTS;

void main() {
    vec4 worldPos = scene.model * vec4(inPosition, 1.0);
    fragWorldPos  = worldPos.xyz;

    mat3 normalMat = transpose(inverse(mat3(scene.model)));
    fragNormal     = normalize(normalMat * inNormal);
    fragTangent    = normalize(normalMat * inTangent.xyz);
    fragBitangent  = cross(fragNormal, fragTangent) * inTangent.w;

    fragTexCoord  = inTexCoord;
    fragTexCoord2 = inTexCoord2;
    fragViewDir   = normalize(scene.cameraPos - worldPos.xyz);

    mat3 TBN = mat3(fragTangent, fragBitangent, fragNormal);
    fragViewDirTS = normalize(transpose(TBN) * fragViewDir);

    gl_Position = scene.projection * scene.view * worldPos;
}
)SHADER_S3_FEATUREHEAVY_VERT";

static const char* k_s3_featureheavy_frag = R"SHADER_S3_FEATUREHEAVY_FRAG(
#version 450

// ──── 8 Static switches ────
// #define ENABLE_CLEAR_COAT
// #define ENABLE_SSS
// #define ENABLE_ANISOTROPY
// #define ENABLE_PARALLAX
// #define ENABLE_EMISSIVE
// #define ENABLE_DETAIL_NORMAL
// #define ENABLE_HEIGHT_BLEND
// #define ENABLE_CURVATURE

layout(set = 0, binding = 0) uniform SceneUniforms {
    mat4  model;
    mat4  view;
    mat4  projection;
    vec4  cameraPos;
    vec4  lightDir;
    vec4  lightColor;
    vec4  ambientColor;
    vec4  pointLightPos[4];
    vec4  pointLightColor[4];
    vec4  clearCoatParams;
    vec4  sssParams;
    vec4  anisotropyParams;
    vec4  parallaxParams;
} scene;

// 16 texture bindings
layout(set = 0, binding = 1)  uniform sampler2D texAlbedo;
layout(set = 0, binding = 2)  uniform sampler2D texNormal;
layout(set = 0, binding = 3)  uniform sampler2D texRoughness;
layout(set = 0, binding = 4)  uniform sampler2D texMetallic;
layout(set = 0, binding = 5)  uniform sampler2D texAO;
layout(set = 0, binding = 6)  uniform sampler2D texEmissive;
layout(set = 0, binding = 7)  uniform sampler2D texHeight;
layout(set = 0, binding = 8)  uniform sampler2D texClearCoatNormal;
layout(set = 0, binding = 9)  uniform sampler2D texClearCoatRoughness;
layout(set = 0, binding = 10) uniform sampler2D texSubsurface;
layout(set = 0, binding = 11) uniform sampler2D texAnisotropyDir;
layout(set = 0, binding = 12) uniform sampler2D texDetailNormal;
layout(set = 0, binding = 13) uniform sampler2D texDetailAlbedo;
layout(set = 0, binding = 14) uniform sampler2D texCurvature;
layout(set = 0, binding = 15) uniform sampler2D texThickness;
layout(set = 0, binding = 16) uniform sampler2D texBRDFLut;

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragTangent;
layout(location = 4) in vec3 fragBitangent;
layout(location = 5) in vec3 fragViewDir;

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;

// ──────── PBR core ────────

float distributionGGX(float NdotH, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float d = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}

// Anisotropic GGX distribution
float distributionGGXAniso(float NdotH, float TdotH, float BdotH, float ax, float ay) {
    float d = (TdotH * TdotH) / (ax * ax) + (BdotH * BdotH) / (ay * ay) + NdotH * NdotH;
    return 1.0 / (PI * ax * ay * d * d);
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float geometrySmith(float NdotV, float NdotL, float roughness) {
    return geometrySchlickGGX(NdotV, roughness) * geometrySchlickGGX(NdotL, roughness);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ──────── Parallax Occlusion Mapping ────────

vec2 parallaxOcclusionMapping(vec2 uv, vec3 viewDirTS) {
    float heightScale = scene.parallaxParams.x;
    float minLayers   = scene.parallaxParams.y;
    float maxLayers   = scene.parallaxParams.z;

    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDirTS)));
    float layerDepth = 1.0 / numLayers;
    float currentLayerDepth = 0.0;

    vec2 P = viewDirTS.xy / viewDirTS.z * heightScale;
    vec2 deltaUV = P / numLayers;

    vec2  currentUV    = uv;
    float currentDepth = texture(texHeight, currentUV).r;

    for (int i = 0; i < 32; i++) {
        if (currentLayerDepth >= currentDepth) break;
        currentUV -= deltaUV;
        currentDepth = texture(texHeight, currentUV).r;
        currentLayerDepth += layerDepth;
    }

    // Interpolate between previous and current for occlusion
    vec2  prevUV    = currentUV + deltaUV;
    float afterDepth  = currentDepth - currentLayerDepth;
    float beforeDepth = texture(texHeight, prevUV).r - currentLayerDepth + layerDepth;
    float weight = afterDepth / (afterDepth - beforeDepth);

    return mix(currentUV, prevUV, weight);
}

// ──────── Subsurface scattering approx ────────

vec3 subsurfaceScattering(vec3 N, vec3 L, vec3 V, vec3 subsurfColor, float thickness, float curvature) {
    float wrap = scene.sssParams.x;
    float wrapDiffuse = max(0.0, (dot(N, L) + wrap) / (1.0 + wrap));
    wrapDiffuse *= wrapDiffuse;

    // Back-lighting through thin surfaces
    vec3  H_sss     = normalize(L + N * 0.5);
    float VdotH_sss = pow(clamp(dot(V, -H_sss), 0.0, 1.0), 4.0);
    float backLight  = VdotH_sss * thickness;

    // Curvature-based ambient
    float curvAmbient = curvature * scene.sssParams.z;

    return subsurfColor * (wrapDiffuse + backLight + curvAmbient);
}

// ──────── Evaluate a single light ────────

vec3 evaluateLight(
    vec3 N, vec3 V, vec3 T, vec3 B,
    vec3 L, vec3 lightRadiance,
    vec3 albedo, float roughness, float metallic,
    vec3 F0, float anisotropy,
    vec3 subsurfColor, float thickness, float curvature
) {
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.001);
    float NdotH = max(dot(N, H), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    // Specular
    float D;
#ifdef ENABLE_ANISOTROPY
    if (anisotropy > 0.001) {
        float TdotH = dot(T, H);
        float BdotH = dot(B, H);
        float aspect = sqrt(1.0 - 0.9 * anisotropy);
        float ax = max(0.001, roughness * roughness / aspect);
        float ay = max(0.001, roughness * roughness * aspect);
        D = distributionGGXAniso(NdotH, TdotH, BdotH, ax, ay);
    } else {
        D = distributionGGX(NdotH, roughness);
    }
#else
    D = distributionGGX(NdotH, roughness);
#endif

    float G = geometrySmith(NdotV, NdotL, roughness);
    vec3  F = fresnelSchlick(HdotV, F0);

    vec3 spec = (D * G * F) / (4.0 * NdotV * NdotL + 0.0001);
    vec3 kD = (1.0 - F) * (1.0 - metallic);

    vec3 result = (kD * albedo / PI + spec) * lightRadiance * NdotL;

    // Subsurface
#ifdef ENABLE_SSS
    if (thickness > 0.0) {
        vec3 sss = subsurfaceScattering(N, L, V, subsurfColor, thickness, curvature);
        result += sss * lightRadiance * 0.25;
    }
#endif

    return result;
}

void main() {
    vec3 viewDirTS = normalize(fragViewDir);
    vec2 uv = fragTexCoord;

    // ──── Parallax (deep nesting level 1) ────
#ifdef ENABLE_PARALLAX
    uv = parallaxOcclusionMapping(uv, viewDirTS);
    // Discard if UV out of range (nesting level 2)
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        discard;
    }
#endif

    // ──── 16 texture samples ────
    vec4  albedoSample = texture(texAlbedo, uv);
    vec3  normalSample = texture(texNormal, uv).rgb;
    float roughness    = texture(texRoughness, uv).r;
    float metallic     = texture(texMetallic, uv).r;
    float ao           = texture(texAO, uv).r;
    vec3  emissive     = texture(texEmissive, uv).rgb;
    float height       = texture(texHeight, uv).r;

    vec3  ccNormal     = texture(texClearCoatNormal, uv).rgb;
    float ccRoughness  = texture(texClearCoatRoughness, uv).r;
    vec3  subsurfColor = texture(texSubsurface, uv).rgb;
    vec3  anisoDir     = texture(texAnisotropyDir, uv).rgb;
    vec3  detailNormal = texture(texDetailNormal, uv * 4.0).rgb;
    vec3  detailAlbedo = texture(texDetailAlbedo, uv * 4.0).rgb;
    float curvature    = texture(texCurvature, uv).r;
    float thickness    = texture(texThickness, uv).r;
    vec2  brdfLut      = texture(texBRDFLut, vec2(0.5, roughness)).rg;

    vec3 albedo = pow(albedoSample.rgb, vec3(2.2));

    // Detail blending (nesting level 2)
#ifdef ENABLE_DETAIL_NORMAL
    albedo *= mix(vec3(1.0), detailAlbedo, 0.3);
#endif

    // ──── Normal mapping ────
    vec3 N = normalize(fragNormal);
    vec3 T = normalize(fragTangent);
    vec3 B = normalize(fragBitangent);
    mat3 TBN = mat3(T, B, N);

    vec3 nrmTS = normalSample * 2.0 - 1.0;

#ifdef ENABLE_DETAIL_NORMAL
    // Blend detail normal via UDN method (nesting level 3)
    vec3 detailTS = detailNormal * 2.0 - 1.0;
    nrmTS = normalize(vec3(nrmTS.xy + detailTS.xy, nrmTS.z));
#endif

    N = normalize(TBN * nrmTS);

    // Anisotropy tangent reorientation
    float anisotropy = 0.0;
#ifdef ENABLE_ANISOTROPY
    anisotropy = scene.anisotropyParams.x;
    float anisoRot = scene.anisotropyParams.y;
    vec3 anisoTangent = anisoDir * 2.0 - 1.0;
    // Rotate tangent in tangent space (nesting level 3)
    float cosR = cos(anisoRot);
    float sinR = sin(anisoRot);
    T = normalize(T * cosR + B * sinR);
    B = cross(N, T);
#endif

    // ──── Lighting ────
    vec3 V  = normalize(scene.cameraPos.xyz - fragWorldPos);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 Lo = vec3(0.0);

    // Directional light
    {
        vec3 L = normalize(-scene.lightDir.xyz);
        vec3 radiance = scene.lightColor.rgb * scene.lightDir.w;
        Lo += evaluateLight(N, V, T, B, L, radiance, albedo, roughness, metallic,
                            F0, anisotropy, subsurfColor, thickness, curvature);
    }

    // 4 point lights (nesting level 4-5 with inner branches)
    for (int i = 0; i < 4; i++) {
        vec3  lightPos   = scene.pointLightPos[i].xyz;
        vec3  lightColor = scene.pointLightColor[i].rgb;
        float lightRange = scene.pointLightColor[i].w;

        vec3  diff     = lightPos - fragWorldPos;
        float dist     = length(diff);
        vec3  L        = diff / dist;

        // Attenuation with range check (nesting level 5)
        if (dist < lightRange) {
            float attenuation = 1.0 / (dist * dist + 1.0);
            attenuation *= clamp(1.0 - (dist / lightRange), 0.0, 1.0);

            vec3 radiance = lightColor * attenuation;
            Lo += evaluateLight(N, V, T, B, L, radiance, albedo, roughness, metallic,
                                F0, anisotropy, subsurfColor, thickness, curvature);
        }
    }

    // ──── Clear coat (nesting level 3) ────
#ifdef ENABLE_CLEAR_COAT
    {
        float ccIntensity = scene.clearCoatParams.x;
        float ccRough     = ccRoughness * scene.clearCoatParams.y;

        vec3 ccN = normalize(TBN * (ccNormal * 2.0 - 1.0));
        vec3 L   = normalize(-scene.lightDir.xyz);
        vec3 H   = normalize(V + L);

        float ccNdotH = max(dot(ccN, H), 0.0);
        float ccNdotL = max(dot(ccN, L), 0.0);
        float ccNdotV = max(dot(ccN, V), 0.001);
        float ccHdotV = max(dot(H, V), 0.0);

        float ccD = distributionGGX(ccNdotH, ccRough);
        float ccG = geometrySmith(ccNdotV, ccNdotL, ccRough);
        vec3  ccF = fresnelSchlick(ccHdotV, vec3(0.04));

        vec3 ccSpec = (ccD * ccG * ccF) / (4.0 * ccNdotV * ccNdotL + 0.0001);
        Lo += ccSpec * scene.lightColor.rgb * scene.lightDir.w * ccNdotL * ccIntensity;

        // Energy conservation: attenuate base layer
        Lo *= (1.0 - ccF * ccIntensity);
    }
#endif

    // ──── Ambient ────
    vec3 ambient = scene.ambientColor.rgb * albedo * ao;

    // ──── Emissive ────
    vec3 emissiveContrib = vec3(0.0);
#ifdef ENABLE_EMISSIVE
    emissiveContrib = emissive * 2.0;   // HDR emissive
#endif

    // ──── Curvature-based AO enhancement ────
#ifdef ENABLE_CURVATURE
    ao *= mix(1.0, 0.5, curvature);
#endif

    vec3 color = ambient + Lo + emissiveContrib;

    // Tone map (ACES approximation)
    color = (color * (2.51 * color + 0.03)) / (color * (2.43 * color + 0.59) + 0.14);
    color = pow(clamp(color, 0.0, 1.0), vec3(1.0 / 2.2));

    outColor = vec4(color, 1.0);
}
)SHADER_S3_FEATUREHEAVY_FRAG";

static const char* k_s3_featureheavy_metal = R"SHADER_S3_FEATUREHEAVY_METAL(
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
)SHADER_S3_FEATUREHEAVY_METAL";

static const char* k_s4_permutationstress_vert = R"SHADER_S4_PERMUTATIONSTRESS_VERT(
#version 450

layout(set = 0, binding = 0) uniform SceneUniforms {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 cameraPos;
    vec4 lightDir;
    vec4 lightColor;
    vec4 ambientColor;
    vec4 fogParams;       // x = start, y = end, z = density, w = mode (0=linear,1=exp)
    vec4 shadowMatrix0;   // row 0 of light-space matrix
    vec4 shadowMatrix1;
    vec4 shadowMatrix2;
    vec4 shadowMatrix3;
} scene;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragTangent;
layout(location = 4) out vec3 fragBitangent;
layout(location = 5) out vec4 fragShadowCoord;
layout(location = 6) out float fragViewDist;

void main() {
    vec4 worldPos = scene.model * vec4(inPosition, 1.0);
    fragWorldPos = worldPos.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(scene.model)));
    fragNormal    = normalize(normalMatrix * inNormal);
    fragTangent   = normalize(normalMatrix * inTangent.xyz);
    fragBitangent = cross(fragNormal, fragTangent) * inTangent.w;

    fragTexCoord = inTexCoord;

    // Shadow map coordinate
    mat4 shadowMat = mat4(
        scene.shadowMatrix0,
        scene.shadowMatrix1,
        scene.shadowMatrix2,
        scene.shadowMatrix3
    );
    fragShadowCoord = shadowMat * worldPos;

    // Distance from camera for fog
    fragViewDist = length(scene.cameraPos.xyz - worldPos.xyz);

    gl_Position = scene.projection * scene.view * worldPos;
}
)SHADER_S4_PERMUTATIONSTRESS_VERT";

static const char* k_s4_permutationstress_frag = R"SHADER_S4_PERMUTATIONSTRESS_FRAG(
#version 450

// 6 binary toggles = 64 permutations
// #define HAS_NORMAL_MAP
// #define HAS_EMISSIVE
// #define HAS_AO
// #define USE_IBL
// #define USE_SHADOWS
// #define USE_FOG

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragTangent;
layout(location = 4) in vec3 fragBitangent;
layout(location = 5) in vec3 fragViewDir;
layout(location = 6) in vec4 fragShadowCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform SceneUBO {
    mat4  model;
    mat4  view;
    mat4  projection;
    vec3  cameraPos;
    float _pad0;
} scene;

layout(set = 0, binding = 1) uniform MaterialUBO {
    vec3  lightDir;
    float lightIntensity;
    vec3  lightColor;
    float ambientIntensity;
    float roughnessScale;
    float metallicScale;
    float fogStart;
    float fogEnd;
    vec3  fogColor;
    float fogDensity;
    float shadowBias;
    float iblIntensity;
    float _pad1;
    float _pad2;
} mat_ubo;

layout(set = 1, binding = 0) uniform sampler2D texAlbedo;
layout(set = 1, binding = 1) uniform sampler2D texRoughness;
layout(set = 1, binding = 2) uniform sampler2D texMetallic;

#ifdef HAS_NORMAL_MAP
layout(set = 1, binding = 3) uniform sampler2D texNormal;
#endif

#ifdef HAS_EMISSIVE
layout(set = 1, binding = 4) uniform sampler2D texEmissive;
#endif

#ifdef HAS_AO
layout(set = 1, binding = 5) uniform sampler2D texAO;
#endif

#ifdef USE_IBL
layout(set = 1, binding = 6) uniform samplerCube texIrradiance;
layout(set = 1, binding = 7) uniform samplerCube texPrefiltered;
layout(set = 1, binding = 8) uniform sampler2D   texBRDFLut;
#endif

#ifdef USE_SHADOWS
layout(set = 1, binding = 9) uniform sampler2DShadow texShadowMap;
#endif

const float PI = 3.14159265359;

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a  = roughness * roughness;
    float a2 = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float d = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d + 0.0001);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return (NdotV / (NdotV * (1.0 - k) + k)) *
           (NdotL / (NdotL * (1.0 - k) + k));
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) *
           pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

#ifdef USE_SHADOWS
float sampleShadow(vec4 shadowCoord) {
    vec3 projCoords = shadowCoord.xyz / shadowCoord.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0) return 1.0;

    float shadow = 0.0;
    vec2 texelSize = vec2(1.0 / 2048.0);
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            vec3 samplePos = vec3(projCoords.xy + vec2(x, y) * texelSize,
                                  projCoords.z - mat_ubo.shadowBias);
            shadow += texture(texShadowMap, samplePos);
        }
    }
    return shadow / 9.0;
}
#endif

void main() {
    vec4 albedoSample = texture(texAlbedo, fragTexCoord);
    vec3 albedo       = pow(albedoSample.rgb, vec3(2.2));
    float roughness   = texture(texRoughness, fragTexCoord).r * mat_ubo.roughnessScale;
    float metallic    = texture(texMetallic, fragTexCoord).r * mat_ubo.metallicScale;

    vec3 N;
#ifdef HAS_NORMAL_MAP
    vec3 normalTS = texture(texNormal, fragTexCoord).rgb * 2.0 - 1.0;
    mat3 TBN = mat3(normalize(fragTangent),
                     normalize(fragBitangent),
                     normalize(fragNormal));
    N = normalize(TBN * normalTS);
#else
    N = normalize(fragNormal);
#endif

    vec3 V = normalize(fragViewDir);

    vec3 emissive = vec3(0.0);
#ifdef HAS_EMISSIVE
    emissive = texture(texEmissive, fragTexCoord).rgb;
#endif

    float ao = 1.0;
#ifdef HAS_AO
    ao = texture(texAO, fragTexCoord).r;
#endif

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 L = normalize(-mat_ubo.lightDir);
    vec3 H = normalize(V + L);

    float NDF = distributionGGX(N, H, roughness);
    float G   = geometrySmith(N, V, L, roughness);
    vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3  spec = (NDF * G * F) /
                 (4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001);
    vec3  kD = (vec3(1.0) - F) * (1.0 - metallic);

    float NdotL = max(dot(N, L), 0.0);
    vec3  radiance = mat_ubo.lightColor * mat_ubo.lightIntensity;
    vec3  Lo = (kD * albedo / PI + spec) * radiance * NdotL;

#ifdef USE_SHADOWS
    float shadowFactor = sampleShadow(fragShadowCoord);
    Lo *= shadowFactor;
#endif

    vec3 ambient = vec3(0.0);
#ifdef USE_IBL
    vec3 kS_ibl = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kD_ibl = (vec3(1.0) - kS_ibl) * (1.0 - metallic);

    vec3 irradiance = texture(texIrradiance, N).rgb;
    vec3 diffuseIBL = irradiance * albedo;

    vec3 R = reflect(-V, N);
    float MAX_LOD = 4.0;
    vec3 prefilteredColor = textureLod(texPrefiltered, R, roughness * MAX_LOD).rgb;
    vec2 envBRDF = texture(texBRDFLut, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specularIBL = prefilteredColor * (kS_ibl * envBRDF.x + envBRDF.y);

    ambient = (kD_ibl * diffuseIBL + specularIBL) * mat_ubo.iblIntensity * ao;
#else
    ambient = vec3(mat_ubo.ambientIntensity) * albedo * ao;
#endif

    vec3 color = ambient + Lo + emissive;

#ifdef USE_FOG
    float dist     = length(scene.cameraPos - fragWorldPos);
    float fogLerp  = clamp((dist - mat_ubo.fogStart) / (mat_ubo.fogEnd - mat_ubo.fogStart), 0.0, 1.0);
    float fogExp   = 1.0 - exp(-mat_ubo.fogDensity * dist);
    float fogFinal = max(fogLerp, fogExp);
    color = mix(color, mat_ubo.fogColor, fogFinal);
#endif

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, albedoSample.a);
}
)SHADER_S4_PERMUTATIONSTRESS_FRAG";

static const char* k_s4_permutationstress_metal = R"SHADER_S4_PERMUTATIONSTRESS_METAL(
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
)SHADER_S4_PERMUTATIONSTRESS_METAL";

ShaderSource load_shader_source(const std::string& tier) {
    if (tier == "S1_BasePBR") {
        return { k_s1_basepbr_vert, k_s1_basepbr_frag, k_s1_basepbr_metal };
    } else if (tier == "S2_MaterialLayered") {
        return { k_s2_materiallayered_vert, k_s2_materiallayered_frag, k_s2_materiallayered_metal };
    } else if (tier == "S3_FeatureHeavy") {
        return { k_s3_featureheavy_vert, k_s3_featureheavy_frag, k_s3_featureheavy_metal };
    } else if (tier == "S4_PermutationStress") {
        return { k_s4_permutationstress_vert, k_s4_permutationstress_frag, k_s4_permutationstress_metal };
    }
    return {};
}

} // namespace benchmark

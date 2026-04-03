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

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

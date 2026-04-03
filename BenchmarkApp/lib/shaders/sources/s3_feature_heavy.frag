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

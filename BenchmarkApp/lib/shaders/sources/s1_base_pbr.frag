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

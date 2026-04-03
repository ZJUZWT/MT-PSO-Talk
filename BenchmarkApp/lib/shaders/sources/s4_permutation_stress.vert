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

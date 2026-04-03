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

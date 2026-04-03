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

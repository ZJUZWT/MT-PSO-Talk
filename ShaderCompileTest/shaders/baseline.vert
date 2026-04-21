#version 450

layout(location = 0) in vec3 inPos;
layout(location = 0) out vec4 baseColor;

// Same push constant layout as heavy shader for compatibility
layout(push_constant) uniform PushConstants {
    int   loopCount;
    float seed;
} pc;

void main() {
    gl_Position = vec4(inPos, 1.0);
    baseColor = vec4(0.5, 0.5, 0.5, 1.0);  // Simple constant color
}

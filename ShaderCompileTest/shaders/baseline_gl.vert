#version 430 core

layout(location = 0) in vec3 inPos;
out vec4 baseColor;

uniform int   loopCount;   // unused, but kept for uniform compatibility
uniform float seed;        // unused

void main() {
    gl_Position = vec4(inPos, 1.0);
    baseColor = vec4(0.5, 0.5, 0.5, 1.0);  // Simple constant color
}

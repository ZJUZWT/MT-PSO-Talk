#version 450

layout(location = 0) in vec4 baseColor;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = baseColor;   // Simply output the constant color
}

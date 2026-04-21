#version 430 core

in vec4 baseColor;
out vec4 outColor;

void main() {
    outColor = baseColor;   // Simply output the constant color
}

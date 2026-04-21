#version 430 core

in vec4 heavyColor;
out vec4 outColor;

void main() {
    outColor = heavyColor;   // Simply output the VS result
}

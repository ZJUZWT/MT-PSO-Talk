#version 430 core

layout(location = 0) in vec3 inPos;
out vec4 heavyColor;

uniform int   loopCount;   // e.g. 5000
uniform float seed;

void main() {
    gl_Position = vec4(inPos, 1.0);

    vec4 acc = vec4(inPos, 1.0) * seed;

    for (int i = 0; i < loopCount; i++) {
        acc = sin(acc) * cos(acc)
            + exp(acc * 0.01) - log(abs(acc) + 1.0);
    }

    heavyColor = acc;
}

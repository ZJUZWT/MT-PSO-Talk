#version 450

layout(location = 0) in vec3 inPos;
layout(location = 0) out vec4 heavyColor;

// Runtime parameters – compiler cannot constant-fold
layout(push_constant) uniform PushConstants {
    int   loopCount;   // e.g. 5000
    float seed;
} pc;

void main() {
    gl_Position = vec4(inPos, 1.0);

    // Seed from vertex position (per-vertex varying)
    vec4 acc = vec4(inPos, 1.0) * pc.seed;

    // Heavy loop – each iteration depends on the previous
    for (int i = 0; i < pc.loopCount; i++) {
        acc = sin(acc) * cos(acc)                  // transcendental ALU
            + exp(acc * 0.01) - log(abs(acc) + 1.0); // more ALU pressure
    }

    heavyColor = acc;
}

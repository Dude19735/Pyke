#version 450

layout(location = 0) in vec4 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
//    outColor = fragColor;
    outColor = vec4(0.0, 0.9, 0.9, 1.0);
}

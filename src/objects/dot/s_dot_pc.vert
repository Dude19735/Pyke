#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 m;
} mvp;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
// layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec4 fragColor;

//push constants block
//layout(push_constant) uniform constants
//{
//    float width;
//    float alpha;
//    float value3;
//} PushConstants;

void main() {
    gl_PointSize = 5;// PushConstants.width;

    gl_Position = mvp.m * vec4(inPosition, 1.0f);
    fragColor = vec4(inColor, 1.0f);
}

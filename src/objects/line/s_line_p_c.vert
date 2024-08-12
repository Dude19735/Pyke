#version 450

// Buffer associated with the attached renderer (just one overall)
layout(binding = 0) uniform UniformBufferObject1 {
    mat4 pv;
} renderer;

// Buffer associated with the model (one per model)
layout(binding = 1) uniform UniformBufferObject2 {
    mat4 m;
} model;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
// layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec4 fragColor;

//push constants block
layout(push_constant) uniform constants
{
    float alpha;
} PushConstants;

void main() {
    gl_Position = renderer.pv * model.m * vec4(inPosition.x, inPosition.y, inPosition.z, 1.0f);
    fragColor = vec4(inColor, PushConstants.alpha);
}

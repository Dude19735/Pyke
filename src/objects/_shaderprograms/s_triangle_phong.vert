#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
} ubo;

layout(binding = 1) uniform VpUniformBufferObject {
    mat4 vp;
} vpo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec3 vertexPosition;
layout(location = 2) out vec3 interpolatedNormal;

//push constants block
layout(push_constant) uniform constants
{
    float width;
    float alpha;
    float value3;
} PushConstants;

void main() {
    vec4 vertexPosition4 = ubo.model * vec4(inPosition, 1.0);
    gl_Position = vpo.vp * vertexPosition4;
    fragColor = vec4(inColor, PushConstants.alpha);

    // scale the vertex position
    vertexPosition = vec3(vertexPosition4) / vertexPosition4.w;

    // move the normals onto the object and then move the normals relative to the object
    // to where the object is located
    interpolatedNormal = vec3(ubo.model * vec4(vertexPosition + inNormal, 0.0));
}

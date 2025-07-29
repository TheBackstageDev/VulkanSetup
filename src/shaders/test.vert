#version 450

layout(push_constant) uniform PushConstant 
{
    mat4 modelMatrix;
} push;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec3 tangent;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec4 fragColor;

layout(set = 0, binding = 0) uniform globalBuffer {
    mat4 projection;
    mat4 view;
} global;

void main()
{
    gl_Position = (global.projection * global.view) * push.modelMatrix * vec4(position, 1.0f);
    
    fragColor = vec4(color, 1.0f);
    fragNormal = normalize(mat3(push.modelMatrix) * normal);
}

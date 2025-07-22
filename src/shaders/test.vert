#version 450

layout(push_constant) uniform PushConstant 
{
    mat4 modelMatrix;
} push;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec4 fragColor;

void main()
{
    gl_Position = vec4(position, 1.0f) * push.modelMatrix;
    fragColor = vec4(color, 1.0f);
}
